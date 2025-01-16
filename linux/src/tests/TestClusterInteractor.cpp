// A mapping from a cluster to its device name.
#include "matter_device_types_clusters_list.inc"

// Unify bridge components
#include "cluster_emulator.hpp"
#include "matter_cluster_interactor.hpp"
#include "matter_device_translator.hpp"
#include "matter_endpoint_builder.hpp"

// Chip components
// // #include <lib/support/UnitTestContext.h>
// // #include <lib/support/UnitTestRegistration.h>

// Third party library
#include <gtest/gtest.h>
#include <pw_unit_test/framework.h>

#define TEST_LOG_TAG "ClusterInteractorTest"
namespace chip {
namespace app {
namespace TestPath {
    
TEST(TestClusterInteractor, TestClusterInteractorNotYetBuilt)
{
    // Initialize the cluster interactor
    unify::matter_bridge::ClusterEmulator emulator;
    unify::matter_bridge::device_translator matter_device_translator(false);
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(emulator, matter_device_translator, builder);

    const char * dt =
        matter_device_type_vs_clusters_map.find(cluster_interactor.get_matter_type().value())->second.device_type_name;
    std::string device_type(dt);
    EXPECT_EQ(device_type,"pressuresensor");
}

TEST(TestClusterInteractor, TestClusterInteractorEmpty)
{
    // Initialize the cluster interactor
    unify::matter_bridge::device_translator matter_device_translator(false);
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::ClusterEmulator emulator;

    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(emulator, matter_device_translator, builder);

    std::unordered_map<std::string, unify::node_state_monitor::cluster> clusters = {};
    cluster_interactor.build_matter_cluster(clusters);
    const char * dt =
        matter_device_type_vs_clusters_map.find(cluster_interactor.get_matter_type().value())->second.device_type_name;
    std::string device_type(dt);
    EXPECT_EQ(device_type, "pressuresensor");
}

TEST(TestClusterInteractor, TestClusterInteractorDoorLock)
{
    // Initialize the cluster interactor
    unify::matter_bridge::device_translator matter_device_translator(false);
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::ClusterEmulator emulator;

    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(emulator, matter_device_translator, builder);

    std::unordered_map<std::string, unify::node_state_monitor::cluster> clusters = {
        { "Basic", unify::node_state_monitor::cluster("Basic") },
        { "Identify", unify::node_state_monitor::cluster("Identify") },
        { "DoorLock", unify::node_state_monitor::cluster("DoorLock") },
        { "TimeSynchronization", unify::node_state_monitor::cluster("TimeSynchronization") },
        { "Binding", unify::node_state_monitor::cluster("Binding") },
    };

    for (auto & [_matter, unify] : clusters)
    {
        unify.attributes.emplace("@");
    }

    cluster_interactor.build_matter_cluster(clusters);
    const char * dt =
        matter_device_type_vs_clusters_map.find(cluster_interactor.get_matter_type().value())->second.device_type_name;
    std::string device_type(dt);
    EXPECT_EQ(device_type,"doorlock");
}

TEST(TestClusterInteractor, TestClusterInteractorOccupancySensor)
{
    // Initialize the cluster interactor
    unify::matter_bridge::ClusterEmulator emulator;
    unify::matter_bridge::device_translator matter_device_translator(false);
    unify::matter_bridge::matter_endpoint_builder builder;
    unify::matter_bridge::cluster_interactor cluster_interactor =
        unify::matter_bridge::cluster_interactor(emulator, matter_device_translator, builder);

    std::unordered_map<std::string, unify::node_state_monitor::cluster> clusters = {
        { "Basic", unify::node_state_monitor::cluster("Basic") },
        { "Identify", unify::node_state_monitor::cluster("Identify") },
        { "OccupancySensing", unify::node_state_monitor::cluster("OccupancySensing") },
        { "IASZone", unify::node_state_monitor::cluster("IASZone") },
    };

    for (auto & [_matter, unify] : clusters)
    {
        unify.attributes.emplace("@");
    }

    cluster_interactor.build_matter_cluster(clusters);
    const char * dt =
        matter_device_type_vs_clusters_map.find(cluster_interactor.get_matter_type().value())->second.device_type_name;
    std::string device_type(dt);
    EXPECT_EQ(device_type,"occupancysensor");
}
}
}
}

