#pragma once
#include <vector>
#include <string>
#include <utility>
#include "Candlestick.h"
/**
 * TextPlotter: contains static methods to render ASCII charts:
 *   1) drawCandlesticks(...) – candlestick chart of OHLC data
 *   2) drawVolumeChart(...) – bar chart of volume (timestamp, amount)
 *   3) drawMeanPriceChart(...) – bar chart of average price per minute
 */
class TextPlotter {
public:
    /**
     * Render a text‐based candlestick chart:
     *   - '|' for whisks (if price level between low and high)
     *   - '*' for body (if level between open and close)
     *   - Rows are price levels (20 rows total)
     *   - Columns are successive candles
     */
    // Draw a text‐based candlestick chart
    static void drawCandlesticks(const std::vector<Candlestick>& candles);
    /**
         * Render a text‐based bar chart of volume:
         *   - For each (timestamp, amount): draw `len = (amount / maxAmount)*50` stars
         */
    // Draw a text‐based bar chart of volume (timestamp, amount)
    static void drawVolumeChart(
  const std::vector<std::pair<std::string,double>>& vol);
    /**
     * Render a text‐based bar chart of mean price per minute:
     *   - Input: vector of (minuteLabel, avgPrice)
     *   - Normalize across the day’s [minAvg, maxAvg], then scale to 0..50 stars
     */
    static void drawMeanPriceChart(const std::vector<std::pair<std::string, double>>& prices);

};
