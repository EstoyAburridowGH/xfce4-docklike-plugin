#ifndef UNITYLAUNCHER_HPP
#define UNITYLAUNCHER_HPP

#include <functional>
#include <string>
#include <cstdint>

namespace UnityLauncher
{
	void init();
	void finalize();

	using UpdateCallback = std::function<void(const std::string& desktopId, int64_t count, bool visible)>;
	void setUpdateCallback(UpdateCallback cb);
} // namespace UnityLauncher

#endif // UNITYLAUNCHER_HPP
