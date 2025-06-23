const Alice2UI = {
    init() {
        console.log('Initializing Alice 2 UI...');

        if (typeof Module === 'undefined') {
            setTimeout(() => this.init(), 100);
            return;
        }

        // Wait for Alice 2 functions to be available
        if (!this.checkAlice2Functions()) {
            setTimeout(() => this.init(), 100);
            return;
        }

        this.setupEventListeners();
        this.setupPerformanceMonitoring();
        this.handleResize();

        console.log('Alice 2 UI initialized successfully');
    },

    checkAlice2Functions() {
        // Check if Alice 2 exported functions are available
        const requiredFunctions = [
            '_alice2_set_background_brightness',
            '_alice2_add_test_geometry',
            '_alice2_clear_scene',
            '_alice2_resize'
        ];

        return requiredFunctions.every(func => typeof Module[func] === 'function');
    },

    handleResize() {
        const canvas = document.getElementById('canvas');
        const container = document.getElementById('canvas-container');

        if (!canvas || !container) return;

        const rect = container.getBoundingClientRect();

        canvas.width = rect.width * window.devicePixelRatio;
        canvas.height = rect.height * window.devicePixelRatio;
        canvas.style.width = rect.width + 'px';
        canvas.style.height = rect.height + 'px';

        console.log('Canvas resized:', canvas.width, 'x', canvas.height);

        // Notify Alice 2 about the resize
        if (Module && Module._alice2_resize) {
            Module._alice2_resize(canvas.width, canvas.height);
        }
    },

    debouncedResize: null,
    performanceData: {
        frameCount: 0,
        lastTime: 0,
        fps: 0
    },

    addSlider(selector, callback, defaultValue = 0.5, valueFormatter = null) {
        const slider = document.querySelector(selector);
        const valueDisplay = slider?.parentElement?.querySelector('.slider-value');

        if (slider && typeof callback === 'function') {
            slider.value = defaultValue;

            const updateValue = (value) => {
                if (valueDisplay) {
                    valueDisplay.textContent = valueFormatter ? valueFormatter(value) : value;
                }
            };

            updateValue(defaultValue);

            slider.addEventListener('input', (e) => {
                const value = parseFloat(e.target.value);
                updateValue(value);

                try {
                    callback(value);
                } catch (error) {
                    console.error(`Error calling slider callback for ${selector}:`, error);
                }
            });
        }
    },

    addButton(selector, callback) {
        const button = document.querySelector(selector);
        if (button && typeof callback === 'function') {
            button.addEventListener('click', () => {
                try {
                    callback();
                } catch (error) {
                    console.error(`Error calling button callback for ${selector}:`, error);
                }
            });
        }
    },

    setupEventListeners() {
        // Scene controls
        this.addButton('#add-test-geometry', () => {
            if (Module._alice2_add_test_geometry) {
                Module._alice2_add_test_geometry();
            }
        });

        this.addButton('#clear-scene', () => {
            if (Module._alice2_clear_scene) {
                Module._alice2_clear_scene();
            }
        });

        this.addButton('#reset-camera', () => {
            if (Module._alice2_reset_camera) {
                Module._alice2_reset_camera();
            }
        });

        // Rendering controls
        this.addSlider('#brightness-slider', (value) => {
            if (Module._alice2_set_background_brightness) {
                Module._alice2_set_background_brightness(value);
            }
        }, 0.2);

        this.addSlider('#point-size-slider', (value) => {
            if (Module._alice2_set_point_size) {
                Module._alice2_set_point_size(value);
            }
        }, 5.0);

        this.addSlider('#line-width-slider', (value) => {
            if (Module._alice2_set_line_width) {
                Module._alice2_set_line_width(value);
            }
        }, 1.0);

        // Camera controls
        this.addSlider('#fov-slider', (value) => {
            if (Module._alice2_set_fov) {
                Module._alice2_set_fov(value);
            }
        }, 45, (v) => `${v}°`);

        // Debug controls
        this.addButton('#toggle-wireframe', () => {
            if (Module._alice2_toggle_wireframe) {
                Module._alice2_toggle_wireframe();
            }
        });

        this.addButton('#performance-info', () => {
            this.showPerformanceInfo();
        });

        // Window resize handling
        window.addEventListener('resize', () => {
            if (this.debouncedResize) {
                clearTimeout(this.debouncedResize);
            }

            this.debouncedResize = setTimeout(() => {
                this.handleResize();
            }, 100);
        });
    },

    setupPerformanceMonitoring() {
        const fpsCounter = document.getElementById('fps-counter');

        const updateFPS = () => {
            const now = performance.now();
            this.performanceData.frameCount++;

            if (now - this.performanceData.lastTime >= 1000) {
                this.performanceData.fps = Math.round(
                    (this.performanceData.frameCount * 1000) / (now - this.performanceData.lastTime)
                );

                if (fpsCounter) {
                    fpsCounter.textContent = `FPS: ${this.performanceData.fps}`;
                }

                this.performanceData.frameCount = 0;
                this.performanceData.lastTime = now;
            }

            requestAnimationFrame(updateFPS);
        };

        requestAnimationFrame(updateFPS);
    },

    showPerformanceInfo() {
        const info = {
            fps: this.performanceData.fps,
            canvas: {
                width: document.getElementById('canvas').width,
                height: document.getElementById('canvas').height
            },
            devicePixelRatio: window.devicePixelRatio,
            webgpu: 'Available'
        };

        alert(`Performance Info:\n${JSON.stringify(info, null, 2)}`);
    }
};

// Initialize Alice 2 UI when the page loads
window.addEventListener('load', () => {
    console.log('Page loaded, setting up Alice 2 UI...');

    window.Module = window.Module || {};
    const existingInit = Module.onRuntimeInitialized;

    Module.onRuntimeInitialized = function() {
        console.log('Module runtime initialized');
        if (existingInit) existingInit();
        Alice2UI.init();
    };

    // Also try to initialize immediately in case Module is already ready
    if (Module.calledRun) {
        Alice2UI.init();
    }
});

// Export for global access
window.Alice2UI = Alice2UI;