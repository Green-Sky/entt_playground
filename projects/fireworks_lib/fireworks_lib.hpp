#pragma once

#include <entt/entity/fwd.hpp>
#include <entt/entity/registry.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/common.hpp>

#include <random>

template<typename RNG>
int64_t random_int(RNG& rng, int64_t min, int64_t max) {
	return (rng() % ((max - min) + 1)) + min;
}

template<typename RNG>
float random_float(RNG& rng, float min, float max, float precision_boost = 100.f) {
	return ((rng() % (size_t((max - min) * precision_boost) + 1)) / precision_boost) + min;
}

namespace Components {
	struct OrgFrameTime {
		// i use "fixedUpdate" and "tick" more or less interchangeable
		float fixedDelta = 1.f/60.f;
	};

	struct Particle2DVel {
		glm::vec2 pos {0.f, 0.f};
		glm::vec2 vel {0.f, 0.f};
		float dampening = 0.f;
	};

	struct ParticleColor {
		glm::vec3 color {1.f, 1.f, 1.f};
	};

	struct ParticleLifeTime {
		float time_remaining = 0.f;
	};

	struct Particle2DPropulsion {
		float dir = 0.f;
		float amount = 10.f; // m/s^2 ?
	};

	// now some fireworks specifics

	struct FireworksExplosion {
		enum exp_type_t : uint16_t {
			FULL = 0,
			FULL_SPLIT, // uses 2
			CIRCLE,
			CIRCLE_2, // uses 2
			exp_type_t_COUNT,
		} type = FULL;

		uint16_t amount = 500;
		uint16_t amount2 = 500;
		float strenth = 80.f;
		float strenth2 = 80.f;
		glm::vec3 color {1.f, 1.f, 1.f};
		glm::vec3 color2 {1.f, 1.f, 1.f};
	};

	struct FireworksRocket {
		float trail_amount = 100.f;
		float trail_amount_accu = 0.f;
		glm::vec3 trail_color {1.f, 0.8f, 0.8f};

		float explosion_timer = 1.f;
		FireworksExplosion explosion;
	};
}

static const size_t color_list_size = 11;
static const glm::vec3 color_list[color_list_size] {
	{1.f, 1.f, 1.f},
	{1.f, 0.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 0.f, 1.f},
	{1.f, 0.f, 1.f},
	{0.f, 1.f, 1.f},
	{1.f, 1.f, 0.f},
	{0.6f, 1.f, 0.f},
	{0.8f, 0.8f, 1.f},
	{0.8f, 0.0f, 1.f},
	{0.3f, 0.9f, 0.3f},
};

static void spawn_fireworks_rocket(entt::registry& scene) {
	auto& mt = scene.ctx<std::mt19937>();

	auto e = scene.create();
	auto& fr = scene.emplace<Components::FireworksRocket>(e);
	fr.trail_amount = 800.f;
	fr.explosion_timer = random_float(mt, 3.5f, 4.5f);

	fr.explosion.amount = 5000;
	fr.explosion.type = Components::FireworksExplosion::exp_type_t(
		random_int(mt, 0, Components::FireworksExplosion::exp_type_t_COUNT-1)
	);
	fr.explosion.color = color_list[random_int(mt, 0, int64_t(color_list_size)-1)];
	fr.explosion.color2 = color_list[random_int(mt, 0, int64_t(color_list_size)-1)];

	auto& p2dv = scene.emplace<Components::Particle2DVel>(e);
	p2dv.pos = {
		random_float(mt, -100.f, 100.f),
		-70.f
	};
	p2dv.vel = {0.f, 0.f};
	p2dv.dampening = 0.5f;

	auto& p2dp = scene.emplace<Components::Particle2DPropulsion>(e);
	//p2dp.amount = 30.f;
	p2dp.amount = random_float(mt, 27.f, 33.f);
	p2dp.dir = (1.5f + random_float(mt, -0.05f, 0.05f)) * glm::pi<float>();
}

static void spawn_fireworks_explosion_full(entt::registry& scene, const glm::vec2& pos, const uint16_t amount, const float strenth, const glm::vec3& color) {
	auto& mt = scene.ctx<std::mt19937>();

	for (size_t i = 0; i < amount; i++) {
		auto e = scene.create();
		{
			auto& part_2d_vel = scene.emplace<Components::Particle2DVel>(e);

			part_2d_vel.pos = pos;

			float dir = random_float(mt, 0.f, 1.f, 10000.f) * glm::two_pi<float>();
			part_2d_vel.vel =
				glm::vec2{glm::cos(dir), glm::sin(dir)}
				* strenth * random_float(mt, 0.f, 1.f, 10000.f);

			part_2d_vel.dampening = 1.0f + random_float(mt, 0.f, 0.5f, 1000.f);
		}

		scene.emplace<Components::ParticleColor>(e, color);

		scene.emplace<Components::ParticleLifeTime>(e, random_float(mt, 0.3f, 5.3f, 1000.f));
	}
}

static void spawn_fireworks_explosion_circle(entt::registry& scene, const glm::vec2& pos, const uint16_t amount, const float strenth, const glm::vec3& color) {
	auto& mt = scene.ctx<std::mt19937>();

	const float dampening = 1.0f + random_float(mt, 0.f, 0.5f, 1000.f);

	for (size_t i = 0; i < amount; i++) {
		auto e = scene.create();
		{
			auto& part_2d_vel = scene.emplace<Components::Particle2DVel>(e);

			part_2d_vel.pos = pos;

			float dir = random_float(mt, 0.f, 1.f, 10000.f) * glm::two_pi<float>();
			part_2d_vel.vel = glm::vec2{glm::cos(dir), glm::sin(dir)} * strenth;

			part_2d_vel.dampening = dampening;
		}

		scene.emplace<Components::ParticleColor>(e, color);

		scene.emplace<Components::ParticleLifeTime>(e, random_float(mt, 0.3f, 5.3f, 1000.f));
	}
}

static void spawn_fireworks_explosion(entt::registry& scene, const glm::vec2& pos, const Components::FireworksExplosion& exp) {
	using exp_type_t = Components::FireworksExplosion::exp_type_t;
	switch (exp.type) {
		case exp_type_t::FULL:
			spawn_fireworks_explosion_full(scene, pos, exp.amount, exp.strenth, exp.color);
			break;
		case exp_type_t::FULL_SPLIT:
			spawn_fireworks_explosion_full(scene, pos, exp.amount, exp.strenth, exp.color);
			spawn_fireworks_explosion_full(scene, pos, exp.amount2, exp.strenth2, exp.color2);
			break;
		case exp_type_t::CIRCLE:
			spawn_fireworks_explosion_circle(scene, pos, exp.amount, exp.strenth, exp.color);
			break;
		case exp_type_t::CIRCLE_2:
			spawn_fireworks_explosion_circle(scene, pos, exp.amount, exp.strenth, exp.color);
			spawn_fireworks_explosion_circle(scene, pos, exp.amount2, exp.strenth2, exp.color2);
			break;
		case exp_type_t::exp_type_t_COUNT:
			break;
	}
}

namespace Systems {

	void particle_fireworks_rocket(
		entt::registry& scene,
		entt::view<entt::exclude_t<>, Components::FireworksRocket, Components::Particle2DPropulsion, const Components::Particle2DVel> view,
		std::mt19937& mt, const Components::OrgFrameTime& ft
	) {
		view.each(
			[&scene, &mt, &ft](
			entt::entity me,
			Components::FireworksRocket& rocket,
			Components::Particle2DPropulsion& rocket_propulsion,
			const Components::Particle2DVel rocket_particle
		) {
			// 1. update timer
			rocket.explosion_timer -= ft.fixedDelta;

			//   ? and explode
			if (rocket.explosion_timer <= 0.f) {
				//// add to delete
				// explode
				spawn_fireworks_explosion(scene, rocket_particle.pos, rocket.explosion);

				// HACK: add empty life to be killed
				scene.emplace<Components::ParticleLifeTime>(me, 0.f);
				// HACK: add new rocket
				spawn_fireworks_rocket(scene);
				return; // early out
			}


			// 2. update propulsion
			float tmp_vel_dir = glm::atan(rocket_particle.vel.y, rocket_particle.vel.x) + glm::pi<float>();
			rocket_propulsion.dir = glm::mix(rocket_propulsion.dir, tmp_vel_dir, 4.5f * ft.fixedDelta);

			// 3. emit trail
			rocket.trail_amount_accu += rocket.trail_amount * ft.fixedDelta;
			while (rocket.trail_amount_accu >= 1.f) {
				rocket.trail_amount_accu -= 1.f;

				auto e = scene.create();

				auto& part_2d_vel = scene.emplace<Components::Particle2DVel>(e);
				part_2d_vel.pos = rocket_particle.pos;

				float dir = rocket_propulsion.dir
					+ random_float(mt, -.1f, .1f, 1000.f); // add a bit of random to trail to simulate cone

				dir = glm::mod<float>(dir + glm::two_pi<float>(), glm::two_pi<float>()); // make positive
				part_2d_vel.vel = glm::vec2{glm::cos(dir), glm::sin(dir)} * rocket_propulsion.amount * 1.5f;

				part_2d_vel.dampening = 5.0f;

				scene.emplace<Components::ParticleColor>(e, rocket.trail_color);

				scene.emplace<Components::ParticleLifeTime>(e, random_float(mt, 0.3f, 0.6f, 1000.f));
			}
		});
	}

	void particle_2d_propulsion(entt::view<entt::exclude_t<>, Components::Particle2DVel, const Components::Particle2DPropulsion> view, const Components::OrgFrameTime& ft) {
		view.each([&ft](Components::Particle2DVel& p, const Components::Particle2DPropulsion& prop) {
			float tmp_dir = prop.dir + glm::pi<float>();
			p.vel +=
				glm::vec2{glm::cos(tmp_dir), glm::sin(tmp_dir)}
				* prop.amount * ft.fixedDelta;
		});
	}

	void particle_2d_gravity(entt::view<entt::exclude_t<>, Components::Particle2DVel> view, const Components::OrgFrameTime& ft) {
		view.each([&ft](Components::Particle2DVel& p) {
			p.vel += glm::vec2{0.f, -10.f} * ft.fixedDelta;
		});
	}

	void particle_2d_vel(entt::view<entt::exclude_t<>, Components::Particle2DVel> view, const Components::OrgFrameTime& ft) {
		view.each([&ft](Components::Particle2DVel& p) {
			p.vel -= p.vel * p.dampening * ft.fixedDelta;
			p.pos += p.vel * ft.fixedDelta;
		});
	}

	void particle_life(entt::view<entt::exclude_t<>, Components::ParticleLifeTime> view, const Components::OrgFrameTime& ft) {
		view.each([&ft](Components::ParticleLifeTime& lt) {
			lt.time_remaining -= ft.fixedDelta;
		});
	}

	//void particle_death(entt::view<entt::exclude_t<>, const Components::ParticleLifeTime> view, entt::registry& scene) {
	void particle_death(entt::registry& scene) {
		auto view = scene.view<Components::ParticleLifeTime>();

		std::vector<entt::entity> to_delete;
		to_delete.reserve(view.size()); // overkill?

		view.each([&to_delete](entt::entity e, const Components::ParticleLifeTime& lt) {
			if (lt.time_remaining <= 0.f) {
				to_delete.push_back(e);
			}
		});

		scene.destroy(to_delete.begin(), to_delete.end());
	}
}

