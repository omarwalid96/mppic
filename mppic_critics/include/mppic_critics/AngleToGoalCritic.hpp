#include <xtensor/xnorm.hpp>
#include <xtensor/xtensor.hpp>
#include <xtensor/xview.hpp>

#include "mppic/utils/common.hpp"
#include "mppic_base/CriticFunction.hpp"

namespace mppi::optimization {

template <typename T>
class AngleToGoalCritic : public CriticFunction<T>
{
public:
  void getParams() final
  {
    auto getParam = utils::getParamGetter(this->parent_, this->node_name_);
    getParam(power_, "angle_to_goal_cost_power", 1);
    getParam(weight_, "angle_to_goal_cost_weight", 15);
  }

  /**
   * @brief Evaluate cost related to robot orientation at goal pose
   * (considered only if robot near last goal in current plan)
   *
   * @param costs [out] add goal angle cost values to this tensor
   */
  virtual void score(
    const geometry_msgs::msg::PoseStamped & robot_pose, const xt::xtensor<T, 3> & trajectories,
    const xt::xtensor<T, 2> & path, xt::xtensor<T, 1> & costs) final
  {
    (void)robot_pose;
    auto init_yaw = tf2::getYaw(robot_pose.pose.orientation);

    auto goal_x = xt::view(path, -1, 0);
    auto goal_y = xt::view(path, -1, 1);
    auto traj_x = xt::view(trajectories, xt::all(), xt::all(), 0);
    auto traj_y = xt::view(trajectories, xt::all(), xt::all(), 1);
    auto traj_yaws = xt::view(trajectories, xt::all(), xt::all(), 2);

    auto angle_between_points = atan2(goal_y - traj_y, goal_x - traj_x);
    auto yaws = xt::abs(traj_yaws - angle_between_points);

    costs += xt::pow(xt::mean(yaws, {1}) * weight_, power_);
  }

private:
  unsigned int power_{0};
  double weight_{0};
};

} // namespace mppi::optimization
