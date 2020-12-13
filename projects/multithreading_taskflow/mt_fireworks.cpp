#include <entt/entity/fwd.hpp>

#include <entt/entity/registry.hpp>
#include <entt/entity/organizer.hpp>

#include <fireworks_lib.hpp>

#include <taskflow/taskflow.hpp>

#include <iostream>
#include <random>
#include <vector>

namespace entt { // inject, i dont remember if this is necessary

// graphviz dot export
static std::ostream& operator<<(std::ostream& out, const std::vector<entt::organizer::vertex>& nodes) {
	out << "digraph EnTT_organizer {\nrankdir=RL;\n";


	for (size_t i = 0; i < nodes.size(); i++) {
		out << "n" << i << "[shape=" << (nodes[i].top_level() ? "doublecircle" : "circle") << " label=\"" << (nodes[i].name() == nullptr ? "NoName" : nodes[i].name()) << "\"];\n";
	}

	for (size_t i = 0; i < nodes.size(); i++) {
		for (const size_t child : nodes[i].children()) {
			out << "n" << child << " -> " << "n" << i << ";\n";
		}
	}

	out << "}";
	return out;
}

} // entt

namespace Systems {

	// this has inner parallelism
	void particle_2d_vel_tf(tf::Subflow* sf, entt::view<entt::exclude_t<>, Components::Particle2DVel> view, const Components::OrgFrameTime& ft) {
		// raw() is only available for groups and single-component-views
		Components::Particle2DVel* raw = view.raw();
		sf->for_each_index_static<size_t, size_t, size_t>(0, view.size(), 1, [&ft, raw](size_t i) {
			auto& p = raw[i];
			p.vel -= p.vel * p.dampening * ft.fixedDelta;
			p.pos += p.vel * ft.fixedDelta;
		}, 0 /* chunk size of zero divides work into worker_count tasks */);
	}

}


void setup_sim(entt::registry& scene) {
	auto& org = scene.ctx_or_set<entt::organizer>(); // we need to keep it around

	//scene.set<std::mt19937>(std::random_device{}());
	scene.set<std::mt19937>(1337 + 42); // since we want it to be predictable
	// ... i know mersenne twister is not good, but its good enough for this test

	{ // systems
		org.emplace<Systems::particle_fireworks_rocket>("particle_fireworks_rocket");
		org.emplace<Systems::particle_2d_propulsion>("particle_2d_propulsion");
		org.emplace<Systems::particle_2d_gravity>("particle_2d_gravity");

		//org.emplace<Systems::particle_2d_vel>("particle_2d_vel"); // this is the not optimsed version

		//tf::Subflow* tmp_sf = nullptr;
		//org.emplace<Systems::particle_2d_vel_tf>(tmp_sf, "particle_2d_vel_tf"); // this is hacky, AND does not buid rn (09.12.2020)

		// This is the method with the most control,
		auto fn = +[](const void* payload, entt::registry& scene) {
			Systems::particle_2d_vel_tf(static_cast<tf::Subflow*>((void*)payload), scene.view<Components::Particle2DVel>(), scene.ctx<Components::OrgFrameTime>());
		};

		// but also with the most responsibility. We have to specify by hand AND overwrite registry as read-only.
		org.emplace<
			const entt::registry, Components::Particle2DVel, const Components::OrgFrameTime
		>(
			fn, nullptr, "particle_2d_vel_tf"
		);

		org.emplace<Systems::particle_life>("particle_life");
		org.emplace<Systems::particle_death>("particle_death");
	}

	// spawn in some rockets to get the simulation running
	spawn_fireworks_rocket(scene);
	spawn_fireworks_rocket(scene);
	spawn_fireworks_rocket(scene);
	spawn_fireworks_rocket(scene);
}

void update_organizer_vertices(entt::registry& scene) {
	scene.ctx_or_set<std::vector<entt::organizer::vertex>>() = scene.ctx_or_set<entt::organizer>().graph();

	if (!scene.try_ctx<Components::OrgFrameTime>()) {
		scene.set<Components::OrgFrameTime>();
	}

	std::cout << "entt organizer graph:\n" << scene.ctx<std::vector<entt::organizer::vertex>>() << "\n";
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;
	tf::Executor ex;

	entt::registry scene;

	setup_sim(scene);

	// this can be a costly operation, so use this sparingly
	update_organizer_vertices(scene);

	tf::Taskflow tf_no_inner;
	tf_no_inner.name("taskflow entt org no-inner-parallelism");
	{ // build taskflow
		// update_organizer_vertices() stores them as ctx
		auto& org_vert = scene.ctx<std::vector<entt::organizer::vertex>>();

		// we need to adress them, before they are finished, so we prepare placeholders
		std::vector<tf::Task> tf_tasks;
		for (size_t i = 0; i < org_vert.size(); i++) { // same size, same indices
			tf_tasks.emplace_back(tf_no_inner.placeholder());
		}

		for (size_t i = 0; i < tf_tasks.size(); i++) {
			auto& v = org_vert[i];
			for (const size_t child : v.children()) {
				tf_tasks[i].precede(tf_tasks[child]); // "copy" dependencies
			}

			tf_tasks[i].work([&scene, &v](tf::Subflow& sf) {
				// this here is somewhat of a hack of the current verion:
				// we suply allways a subflow, and ignore v.data(), this might be very wrong.
				// but its easy callsite injection of the subflow, which we "need" for inner parallalism
				v.callback()(/*v.data()*/(void*)&sf, scene);
			});

			if (v.name()) {
				tf_tasks[i].name(v.name());
			}
		}
	}

	// simulate 1000 ticks
	for (size_t tick = 0; tick < 1000; tick++) {
		// run taskflow on executor
		ex.run(tf_no_inner).wait();
	}

	std::cout << "taskflow graph:\n" << tf_no_inner.dump() << "\n";

	return 0;
}

