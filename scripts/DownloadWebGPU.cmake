# This script checks if webgpu binaries exist, and if not, downloads them.

set(webgpu_bin_dir "${CMAKE_CURRENT_SOURCE_DIR}/depends/webgpu/bin")
file(GLOB bin_contents "${webgpu_bin_dir}/*")

if(NOT bin_contents)
    message(STATUS "WebGPU binaries not found. Downloading for current platform...")

    set(webgpu_bin_url_base "https://github.com/TaizhongC/alice2/releases/download/webgpu_bin")

    if(WIN32)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64")
            set(platform_zip "windows-x86_64.zip")
        endif()
    elseif(APPLE)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
            set(platform_zip "macos-aarch64.zip")
        else()
            set(platform_zip "macos-x86_64.zip")
        endif()
    elseif(UNIX) # Linux
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
            set(platform_zip "linux-aarch64.zip")
        else()
            set(platform_zip "linux-x86_64.zip")
        endif()
    endif()

    if(platform_zip)
        set(download_url "${webgpu_bin_url_base}/${platform_zip}")
        set(zip_location "${webgpu_bin_dir}/${platform_zip}")

        message(STATUS "Downloading from ${download_url}")
        file(DOWNLOAD ${download_url} ${zip_location} STATUS download_status)
        
        list(GET download_status 0 error_code)
        if(error_code EQUAL 0)
            message(STATUS "Download webgpu binaries completed.")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xvf ${zip_location}
                WORKING_DIRECTORY ${webgpu_bin_dir}
            )
            file(REMOVE ${zip_location})
        else()
            list(GET download_status 1 error_message)
            message(FATAL_ERROR "Failed to download ${download_url}: ${error_message}")
        endif()
    else()
        message(FATAL_ERROR "Unsupported platform for automatic WebGPU binaries download.")
    endif()
endif() 