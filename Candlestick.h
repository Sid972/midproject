#pragma once
#include <string>
/**
 * Represents a single “candlestick” in OHLC format.
 *   - timestamp: exact UTC time of this candle
 *   - open: opening price (from previous close or same as close for first candle)
 *   - high: highest price seen during this timeframe
 *   - low:  lowest price seen during this timeframe
 *   - close: VWAP‐style closing price (∑(price*amount)/∑amount)
 */
class Candlestick {
public:
    std::string timestamp;
    double open, high, low, close;
    // Constructor: initialize all four fields and timestamp.
    Candlestick(std::string ts, double o, double h, double l, double c)
      : timestamp(std::move(ts)), open(o), high(h), low(l), close(c) {}
};
