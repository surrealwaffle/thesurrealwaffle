#pragma once

#include <d3d9.h>

#include <functional>

#include <sentutil/utility.hpp>

namespace blamtracer {

enum class RenderResult {
    success,
    error,
    fatal_error
};

using rendering_function = std::function<RenderResult(LPDIRECT3DSURFACE9)>;

class Renderer {
    using install_handle = sentutil::utility::handle;

    LPDIRECT3DSURFACE9 surface = nullptr; ///< Rendering is performed on this surface, then copied to the backbuffer.
    rendering_function render  = nullptr; ///< The rendering implementation. This should not invoke `EndScene()`.
    install_handle     install = nullptr; ///<

public:
    Renderer() = default;
    Renderer(rendering_function&& render);
    Renderer(const Renderer&) = delete;

    Renderer(Renderer&& other);
    Renderer& operator=(Renderer&& other);

    ~Renderer() { release(); }

    bool is_installed() { return surface && render && install; }

    /** \brief Removes this renderer from sentinel and releases associated resources.
     *
     * This function will release resources from any partially constructed state.
     * Renderer implementors must chain the `release()` methods, as if construction took place.
     */
    void release() noexcept;

    /** \brief Attempts to install this renderer.
     *
     * This function does nothing and returns `true` if the
     * renderer is already installed.
     *
     * If the rendering implementation #render is empty, then the install fails.
     *
     * If this function returns `false`, any allocated resources are destroyed with
     * #release.
     *
     * \return `true` if this renderer is installed, otherwise `false`.
     */
    bool try_install();

    /** \brief Attempts to render.
     *
     * If this renderer is not installed, then this function simply returns `false`,
     * with no call to the rendering implementation.
     *
     * If the rendering function returns RenderResult::fatal_error, then the renderer
     * is uninstalled with #release and this function returns `false`. Furthermore,
     * the rendering function itself is cleared, so that
     *
     * \return `true` if rendering was successful, otherwise `false`.
     */
    bool try_render();

    /** \brief Prepares the currently installed renderer (if any) for a device reset.
     */
    static void device_reset(LPDIRECT3DDEVICE9 device);

    /** \brief Recreates device resources that were lost from a reset.
     */
    static void device_acquire(LPDIRECT3DDEVICE9 device,
                               const D3DPRESENT_PARAMETERS* params);
};

}
