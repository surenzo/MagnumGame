//#include "Systems.hpp"
//
//void UpdateInput(entt::registry& registry, Magnum::Vector2& input) {
//    auto view = registry.view<Objet3D>();
//    for (auto entity : view) {
//        auto& obj = view.get<Objet3D>(entity);
//        btVector3 force(input.x() * 5.f, 0, input.y() * 5.f);
//        obj.rigidbody->applyTorque(btVector3(force.z(), 0, -force.x()));
//    }
//}
//
//void UpdateNetwork(entt::registry& registry, Client& client) {
//    client.receive(registry);
//}
//
//void UpdatePhysics(entt::registry& registry, PhysicsSystem& physics, float dt) {
//    physics.stepSimulation(dt, registry);
//}
