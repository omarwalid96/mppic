#define CATCH_CONFIG_RUNNER
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <catch2/catch.hpp>

#include "nav2_costmap_2d/costmap_2d_ros.hpp"
#include <rclcpp/executors.hpp>

#include "geometry_msgs/msg/twist.hpp"
#include "nav_msgs/msg/path.hpp"
#include <geometry_msgs/msg/pose_stamped.hpp>

#include "mppi/Models.hpp"
#include "mppi/impl/Optimizer.hpp"

TEST_CASE("Optimizer evaluates Next Control", "") {
  using T = float;

  std::string node_name = "TestNode";
  auto node = std::make_shared<rclcpp_lifecycle::LifecycleNode>(node_name);
  auto costmap = new nav2_costmap_2d::Costmap2D(500, 500, 0.1, 0, 0, 100);
  auto &model = mppi::models::NaiveModel<T>;

  auto optimizer =
      mppi::optimization::Optimizer<T>(node, node_name, costmap, model);

  optimizer.on_configure();
  optimizer.on_activate();

  size_t poses_count = GENERATE(0, 1, 10, 1000, 10000, 100000);

  WARN("Path with " << poses_count);

  SECTION("Running evalNextControl") {
    geometry_msgs::msg::Twist twist;

    std::string frame = "odom";
    auto time = node->get_clock()->now();
    auto setHeader = [&](auto msg) {
      msg.header.frame_id = frame;
      msg.header.stamp = time;
    };

    nav_msgs::msg::Path path;
    geometry_msgs::msg::PoseStamped ps;
    setHeader(ps);
    setHeader(path);

    auto fillPath = [&](size_t count) {
      for (size_t i = 0; i < count; i++) {
        path.poses.push_back(ps);
      }
    };

    fillPath(poses_count);

    CHECK_NOTHROW(optimizer.evalNextControl(twist, path));
    BENCHMARK("evalNextControl Benchmark") {
      return optimizer.evalNextControl(twist, path);
    };
  }

  optimizer.on_deactivate();
  optimizer.on_cleanup();
  delete costmap;
}


int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  int result = Catch::Session().run(argc, argv);
  return result;
}
