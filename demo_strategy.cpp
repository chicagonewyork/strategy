#include "basics/GlobalDefs.h"
#include "basics/InstrumentId.h"
#include "basics/Price.h"
#include "basics/Time.h"
#include "books/OrderBook.h"
#include "config_tree/ConfigTree.h"
#include "control/ControlCommand.h"

#include "market_data/BarBuilder.h"
#include "strategy_framework/IStrategy.h"
#include "strategy_framework/live/LiveStrategyFramework.h"
#include "strategy_framework/live/StratFrameworkCCProtocol.h"

#include <atomic>
#include <thread>

namespace ma {

using namespace core;

class TestStrategy final : public core::IStrategy {
public:
  TestStrategy() : framework_(nullptr), bb_1min_(core::NANOS_IN_MINUTE), order_mem_pool_("Orders", false), price_level_mem_pool_("PriceLevels", false) {}
private:
  void init(core::IStrategyFramework* framework) final override;
  void onReadyToTrade() final override;
  void onShutdown() final override;
  void onRMDSnapshot(uint64_t listener_id, const std::vector<RMDEvent*>& snapshot) final override;
  void onRMDEvent(uint64_t listener_id, const core::RMDEvent* event) final override;
  void onRMDStatusChange(bool is_up) final override;
  void onOEMsg(uint64_t stream_idx, const core::OEMsg::Hdr* msg) final override;
  void onCommandControl(const core::ControlCommand* cmd, core::ControlCommand* resp) final override;
  void onTimer(int64_t timer_id, core::epoch_ns_t cur_ts) final override;

  IStrategyFramework* framework_;
  
  // 1MINUTE BAR
  instr_id_t instr_id_;
  BarBuilder bb_1min_;
  int64_t timer_id_1min_;
  MemoryPool<OrderBook::Order> order_mem_pool_;
  MemoryPool<OrderBook::PriceLevel> price_level_mem_pool_;
  std::unordered_map<instr_id_t, core::OrderBook> books_;
};

void TestStrategy::init(core::IStrategyFramework* framework) {
  framework_ = framework;
}

void TestStrategy::onReadyToTrade() {
/*
  core::order_id_t order_id2;
  {
    
    const core::Instrument* instr = framework_->getInstrument(core::Exchange::CME, "0DEU4");

    order_id2 = framework_->newOrder(
      instr->getInstrumentId(),
      core::OEMsg::OrderType::Limit,
      core::Side::Sell,
      core::Price::fromDouble(274.80),
      core::Price::UNSET,
      850,
      core::OEMsg::TimeInForce::Day,
      core::OEMsg::LiquidityFlag::Null,
      321);
    INFO("New order has been sent: order_id=%lu", order_id2);
  }
*/
/*
  {
      sleep(2);
      framework_->replaceOrder(order_id2, core::Price::UNSET, core::Price::fromDouble(5439.50), 20, core::OEMsg::OrderType::Stop, core::OEMsg::TimeInForce::Day);
  }

  {
    sleep(2);
    framework_->cancelOrder(order_id2);
  }

  const core::Instrument* instr = framework_->getInstrument(core::Exchange::CME, "ESU4");
  instr_id_ = instr->getInstrumentId();
  bb_1min_.setInstruments({instr_id_});
  timer_id_1min_ = framework_->setRepeatingTimer(Time::getTime() , core::NANOS_IN_MINUTE);
*/
}

void TestStrategy::onShutdown() {
  INFO("Shutting down the strategy");
}

void TestStrategy::onRMDSnapshot(uint64_t listener_id, const std::vector<core::RMDEvent*>& snapshot) {
  std::unordered_map<instr_id_t, OrderBook>::iterator it = books_.end();
  for (size_t i = 0; i < snapshot.size(); ++i) {
    const RMDEvent* ev = snapshot[i];
    if (ev->hdr_.type_ == RMDEvent::EventType::BookSnapshotStart) {
      ASSERT(it == books_.end());
      it = books_.find(ev->hdr_.instr_id_);
      if (it == books_.end()) {
        const Instrument* instr = framework_->getInstrument(ev->hdr_.instr_id_);
        if (instr != nullptr) {  // TODO: Replace with FATAL when instrument service is ready.
          it = books_.emplace(std::piecewise_construct, std::forward_as_tuple(ev->hdr_.instr_id_), std::forward_as_tuple(instr->getInstrumentId(), instr->getTickSize(), &order_mem_pool_, &price_level_mem_pool_)).first;
        }
      }
    } else if (ev->hdr_.type_ == RMDEvent::EventType::BookSnapshotEnd) {
      it = books_.end();
    } else if (it != books_.end()) {
      ASSERT(ev->hdr_.instr_id_ == it->first);
      it->second.processMarketEvent(*ev);
    }
  }
}

void TestStrategy::onRMDEvent(uint64_t listener_id, const core::RMDEvent* ev) {
  std::unordered_map<instr_id_t, OrderBook>::iterator it = books_.find(ev->hdr_.instr_id_);
  if (it == books_.end()) {
    const Instrument* instr = framework_->getInstrument(ev->hdr_.instr_id_);
    if (instr != nullptr) {  // TODO: Replace with FATAL when instrument service is ready.
      it = books_.emplace(std::piecewise_construct, std::forward_as_tuple(ev->hdr_.instr_id_), std::forward_as_tuple(instr->getInstrumentId(), instr->getTickSize(), &order_mem_pool_, &price_level_mem_pool_)).first;
    } else {
      WARN("Instrument %lu is not found in the instrument map", ev->hdr_.instr_id_);
      return;
    }
  }
  it->second.processMarketEvent(*ev);
}

void TestStrategy::onRMDStatusChange(bool is_up) {
  INFO("RMD channel is %s", is_up ? "UP" : "DOWN");
}

void TestStrategy::onTimer(int64_t timer_id, core::epoch_ns_t cur_ts) {  
  if(timer_id == timer_id_1min_){
    bb_1min_.checkTime(cur_ts);
  }

  auto bar = bb_1min_.GetBar(instr_id_);
  INFO("1MINUTE BAR: %s", bar->toString().c_str());
}

void TestStrategy::onCommandControl(const core::ControlCommand* cmd, core::ControlCommand* resp) {
  // TODO: Add custom command processing here.
  INFO("Custom command received: %u", static_cast<uint8_t>(cmd->hdr_.cmd_type_));
  core::StratFrameworkCCProtocol::populateStratResponse(0, "", resp);
}

void TestStrategy::onOEMsg(uint64_t stream_idx, const core::OEMsg::Hdr* msg) {
  INFO("OEMsg received: %s", core::OEMsg::toString(msg).c_str());
}

}  // namespace ma

void printUsage(const char* app_name) {
  printf("%s -- demo strategy\n", app_name);
  printf("Usege: %s <strat_framework_args>\n", app_name);
  printf("{\n");
  printf("  %s\n", ma::core::LiveStrategyFramework::cfgHelpString(2).c_str());
  printf("}\n");
}

int main(int argc, char** argv){
  if (argc == 1) {
    printUsage(argv[0]);
    return EXIT_FAILURE;
  }
  ma::core::ConfigTree cfg_tree(argc, argv);
  INFO("%s", cfg_tree.toString().c_str());
  const ma::core::ConfigTreeNode* root = cfg_tree.getRoot();

  ma::TestStrategy strat;
  ma::core::LiveStrategyFramework strat_framework(root, &strat);
  strat_framework.run();

  return EXIT_SUCCESS;
}