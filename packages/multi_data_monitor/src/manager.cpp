// Copyright 2021 Takagi, Isamu
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "manager.hpp"
#include "matrix.hpp"
#include "simple.hpp"
#include "titled.hpp"
#include <QWidget>
#include <string>

#include <iostream>
#include <ament_index_cpp/get_package_share_directory.hpp>  // TODO: move parser

namespace monitors
{

std::pair<std::string, size_t> parse(const std::string & path, size_t & base)
{
  const size_t pos1 = path.find('(', base);
  const size_t pos2 = path.find(')', base);
  if (pos1 != base + 1 || pos2 == std::string::npos)
  {
    return {"$", 1};
  }
  const auto expr = path.substr(base + 2, pos2 - base - 2);
  if (expr.substr(0, 5) != "find ")
  {
    return {"$", 1};
  }
  return {ament_index_cpp::get_package_share_directory(expr.substr(5)), pos2 - base + 1};
}

void Manager::Load(const std::string & path)
{
  std::string parsed;
  size_t base = 0;
  while (true)
  {
    const size_t pos = path.find('$', base);
    parsed += path.substr(base, pos - base);
    if (pos == std::string::npos)
    {
      break;
    }
    const auto [str, len] = parse(path, base);
    parsed += str;
    base = pos + len;
  }

  yaml_ = YAML::LoadFile(parsed);
  std::cout << "format version: " << yaml_["version"].as<std::string>() << std::endl;
}

void Manager::CreateMonitors()
{
  const auto CreateMonitor = [](const std::string & name, const YAML::Node & yaml) -> std::shared_ptr<Monitor>
  {
    const auto type = yaml["class"].as<std::string>();
    std::cout << "create node: " << name << "  " << type << std::endl;

    if (type == "matrix")
    {
      return std::make_shared<Matrix>(name, yaml);
    }
    if (type == "titled")
    {
      return std::make_shared<Titled>(name, yaml);
    }
    if (type == "simple")
    {
      return std::make_shared<Simple>(name, yaml);
    }

    std::cout << "  unknown type: " << type << std::endl;
    return nullptr;
  };

  for(const auto & monitor : yaml_["monitors"])
  {
    const auto name = monitor.first.as<std::string>();
    monitors_[name] = CreateMonitor(name, monitor.second);
  }
}

void Manager::CreateSubscription(const rclcpp::Node::SharedPtr & node)
{
  std::map<std::string, MonitorList> topics;
  for (const auto & [_, monitor] : monitors_)
  {
    const std::string name = monitor->GetTopicName();
    if (!name.empty())
    {
      topics[name].push_back(monitor);
    }
  }

  for (const auto & topic : topics)
  {
    std::cout << topic.first << std::endl;
    for (const auto & monitor : topic.second)
    {
      std::cout << "  " << monitor->GetName() << std::endl;
    }
    auto subscription = std::make_unique<TopicSubscription>(node, topic.second);
    subscriptions_.push_back(std::move(subscription));
  }
}

void Manager::Build(QWidget * panel)
{
  const auto name = yaml_["root"].as<std::string>();
  const auto root = monitors_.at(name);
  root->Build(monitors_);

  const auto widget = root->GetWidget();
  std::cout << "widget: " << widget << std::endl;
  if (widget)
  {
    // TODO: use dummy layout
    // panel->Widget(widget)
  }

  const auto layout = root->GetLayout();
  std::cout << "layout: " << layout << std::endl;
  if (layout)
  {
    panel->setLayout(layout);
  }
}

}  // namespace monitors
