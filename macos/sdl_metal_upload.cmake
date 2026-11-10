# Backport SDL's Metal upload-stride fix while upstream pins SDL 3.4.16.
# https://github.com/libsdl-org/SDL/commit/32f9c098d597dd29a9cb8cb5d0b5fd6fcc50aca4
# Remove this backport once the pinned SDL includes it. It only changes Metal.
set(file "${SDL3_SOURCE_DIR}/src/gpu/metal/SDL_gpu_metal.m")
file(READ "${file}" text)
set(before [=[        MetalTexture *metalTexture = METAL_INTERNAL_PrepareTextureForWrite(renderer, textureContainer, cycle);

        [metalCommandBuffer->blitEncoder
                 copyFromBuffer:bufferContainer->activeBuffer->handle
                   sourceOffset:source->offset
              sourceBytesPerRow:BytesPerRow(destination->w, textureContainer->header.info.format)
            // sourceBytesPerImage expects the stride between 2D images (slices) of a 3D texture, not the size of the entire region
            sourceBytesPerImage:SDL_CalculateGPUTextureFormatSize(textureContainer->header.info.format, destination->w, destination->h, 1)]=])
set(after [=[        MetalTexture *metalTexture = METAL_INTERNAL_PrepareTextureForWrite(renderer, textureContainer, cycle);

        Uint32 pixelsPerRow = source->pixels_per_row == 0 ? destination->w : source->pixels_per_row;
        Uint32 rowsPerLayer = source->rows_per_layer == 0 ? destination->h : source->rows_per_layer;

        [metalCommandBuffer->blitEncoder
                 copyFromBuffer:bufferContainer->activeBuffer->handle
                   sourceOffset:source->offset
              sourceBytesPerRow:BytesPerRow(pixelsPerRow, textureContainer->header.info.format)
            // sourceBytesPerImage expects the stride between 2D images (slices) of a 3D texture, not the size of the entire region
            sourceBytesPerImage:SDL_CalculateGPUTextureFormatSize(textureContainer->header.info.format, pixelsPerRow, rowsPerLayer, 1)]=])
string(FIND "${text}" "${after}" done)
if(done EQUAL -1)
    string(FIND "${text}" "${before}" at)
    if(at EQUAL -1)
        message(FATAL_ERROR "Mac SDL upload backport no longer matches; check the pinned SDL version")
    endif()
    string(REPLACE "${before}" "${after}" text "${text}")
    file(WRITE "${file}" "${text}")
endif()
