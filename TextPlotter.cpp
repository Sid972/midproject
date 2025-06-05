#include "TextPlotter.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

/**
 * TextPlotter:
 *   Provides static methods to render various data (candlesticks, volume, mean price)
 *   as text-based (ASCII) charts in the console.
 */

/**
 * drawCandlesticks
 * Renders an ASCII candlestick chart for a series of Candlestick objects.
 *
 * @param candles  A vector of Candlestick objects, each containing:
 *                   - timestamp (string, "YYYY/MM/DD HH:MM:SS.ffffff")
 *                   - open, high, low, close (doubles)
 *
 * Behavior:
 *   1. If `candles` is empty, prints "No data to plot" and returns.
 *   2. Determines the global high and low prices across all candles.
 *   3. Divides the price range into a fixed number of rows (20 by default).
 *   4. For each row (top to bottom), computes the price level and:
 *        - Prints the price label on the left (with fixed precision).
 *        - For each candle:
 *            • Prints '*' if the level lies between open and close (the "body").
 *            • Prints '|' if the level lies between low and high but outside the body (the "wick").
 *            • Otherwise prints a space.
 *   5. After drawing all rows, prints a horizontal axis of '-' characters.
 *   6. Prints timestamp labels (HH:MM:SS) every LABEL_EVERY candles beneath the axis.
 */
void TextPlotter::drawCandlesticks(const std::vector<Candlestick>& candles) {
    // 1) Handle empty input
    if (candles.empty()) {
        std::cout << "No data to plot\n";
        return;
    }

    // 2) Determine global high and low across all candles
    double globalHigh = candles.front().high;
    double globalLow  = candles.front().low;
    for (const auto& c : candles) {
        globalHigh = std::max(globalHigh, c.high);
        globalLow  = std::min(globalLow,  c.low);
    }

    // 3) Chart dimensions and scaling
    const int rows = 20;                                // number of horizontal rows
    double rawSpan = globalHigh - globalLow;            // price span
    // If all prices equal, avoid division by zero by using span = 1
    double span = (rawSpan == 0.0 ? 1.0 : rawSpan) / rows;

    // 4) Formatting for price labels on left
    const int PREC = 6;           // show 6 decimal places (e.g., 0.024723)
    const int WID  = PREC + 3;    // enough width to display "-0.xxxxxx"

    // 5) Draw each row from top (highest price) down to bottom (lowest price)
    for (int r = rows; r >= 0; --r) {
        double level = globalLow + r * span;  // price level corresponding to this row

        // Print the price label, right-aligned in a field of width WID
        std::cout << std::setw(WID)
                  << std::fixed << std::setprecision(PREC)
                  << level << " |";

        // For each candle, decide if this level is in the body or wick
        for (const auto& c : candles) {
            bool inWhisker = (c.low <= level && level <= c.high);
            bool inBody    = ((c.open <= level && level <= c.close) ||
                              (c.close <= level && level <= c.open));

            if (inBody)
                std::cout << '*';     // within the candle's body (open-close)
            else if (inWhisker)
                std::cout << '|';     // within the candle's wick (low-high, outside body)
            else
                std::cout << ' ';     // no candle at this level
        }

        std::cout << '\n';  // end of this row
    }

    // 6) Draw the horizontal X-axis using '-' characters
    //    Align under the chart area (WID + 3 spaces for "level |")
    std::cout << std::string(WID + 3, ' ')
              << std::string(candles.size(), '-') << "\n";

    // 7) Print timestamp labels (every LABEL_EVERY candles)
    const size_t LABEL_EVERY = 5;   // print a label for every 5th candle
    const int    LABW        = 8;   // width of "HH:MM:SS" (8 characters)

    // Align under the chart area again
    std::cout << std::string(WID + 3, ' ');
    for (size_t i = 0; i < candles.size(); ++i) {
        if (i % LABEL_EVERY == 0) {
            // Extract the HH:MM:SS portion from "YYYY/MM/DD HH:MM:SS.ffffff"
            std::cout << candles[i].timestamp.substr(11, LABW);
        } else {
            std::cout << std::string(LABW, ' ');
        }
    }
    std::cout << '\n';
}

/**
 * drawVolumeChart
 * Renders a simple text-based bar chart of trading volume over time.
 *
 * @param vol  A vector of (timestamp, volume) pairs, where:
 *               - timestamp: string ("YYYY/MM/DD HH:MM:SS.ffffff")
 *               - volume:     double (sum of amounts at that timestamp)
 *
 * Behavior:
 *   1. If `vol` is empty, prints "No volume data" and returns.
 *   2. Finds the maximum volume value among all pairs.
 *   3. For each (ts, v):
 *        - Computes `len = floor((v / maxV) * 50)`.
 *        - Prints "ts | " followed by `len` asterisks ('*') to represent relative volume.
 *        - Then prints " (v)" showing the actual volume number.
 */
void TextPlotter::drawVolumeChart(
    const std::vector<std::pair<std::string,double>>& vol)
{
    // 1) Handle empty input
    if (vol.empty()) {
        std::cout << "No volume data\n";
        return;
    }

    // 2) Determine the maximum volume value for normalization
    double maxV = 0.0;
    for (auto const& [ts, v] : vol) {
        maxV = std::max(maxV, v);
    }

    // 3) For each timestamp, print a bar of '*' proportional to (v / maxV)
    for (auto const& [ts, v] : vol) {
        int len = static_cast<int>((v / maxV) * 50);  // scale to max 50 stars
        std::cout << ts << " | ";
        for (int i = 0; i < len; ++i) {
            std::cout << '*';
        }
        // Print the actual volume in parentheses
        std::cout << " (" << v << ")\n";
    }
}

/**
 * drawMeanPriceChart
 * Renders a text-based bar chart of average (mean) prices per time bucket (e.g., per minute).
 *
 * @param data  A vector of (timeBucket, avgPrice) pairs, where:
 *                - timeBucket: string like "HH:MM"
 *                - avgPrice:   double (rounded to 6 decimals)
 *
 * Behavior:
 *   1. If `data` is empty, prints "No mean price data." and returns.
 *   2. Finds the minimum and maximum average prices across all buckets.
 *   3. For each (minute, avg):
 *        - Computes `frac = (avg - minP) / (maxP - minP)` in [0,1].
 *        - Computes `len = floor(frac * 50)` to scale into 0–50 stars.
 *        - Prints "minute | " followed by `len` asterisks ('*'), then " (avg)" showing the price.
 *
 * If all average prices are identical (maxP == minP), `span` is set to 1.0 to avoid division by zero.
 */
void TextPlotter::drawMeanPriceChart(
    const std::vector<std::pair<std::string, double>>& data)
{
    // 1) Handle empty input
    if (data.empty()) {
        std::cout << "No mean price data.\n";
        return;
    }

    // 2) Find the global min and max among all average prices
    double minP = data.front().second;
    double maxP = data.front().second;
    for (auto const& [_, avg] : data) {
        minP = std::min(minP, avg);
        maxP = std::max(maxP, avg);
    }

    // Use span = (maxP - minP), or 1.0 if all are equal to avoid division by zero
    double span = (maxP == minP ? 1.0 : (maxP - minP));

    // 3) For each time bucket, normalize and print a bar of '*' proportional to its position in [minP,maxP]
    for (auto const& [minute, avg] : data) {
        double frac = (avg - minP) / span;                 // normalized [0,1]
        int len    = static_cast<int>(frac * 50);          // scale to [0,50]
        std::cout << minute << " | ";
        for (int i = 0; i < len; ++i) {
            std::cout << '*';
        }
        // Print the actual average price to 6 decimal places
        std::cout << " (" << std::fixed << std::setprecision(6) << avg << ")\n";
    }
}
