/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "DynamicImage.h"
#include "gfxPlatform.h"
#include "gfxUtils.h"
#include "mozilla/gfx/2D.h"
#include "mozilla/RefPtr.h"
#include "ImageRegion.h"
#include "Orientation.h"
#include "SVGImageContext.h"

#include "mozilla/MemoryReporting.h"

using namespace mozilla;
using namespace mozilla::gfx;
using mozilla::layers::LayerManager;
using mozilla::layers::ImageContainer;

namespace mozilla {
namespace image {

// Inherited methods from Image.

nsresult
DynamicImage::Init(const char* aMimeType, uint32_t aFlags)
{
  return NS_OK;
}

already_AddRefed<ProgressTracker>
DynamicImage::GetProgressTracker()
{
  return nullptr;
}

nsIntRect
DynamicImage::FrameRect(uint32_t aWhichFrame)
{
  gfxIntSize size(mDrawable->Size());
  return nsIntRect(0, 0, size.width, size.height);
}

size_t
DynamicImage::SizeOfSourceWithComputedFallback(MallocSizeOf aMallocSizeOf) const
{
  return 0;
}

size_t
DynamicImage::SizeOfDecoded(gfxMemoryLocation aLocation,
                            MallocSizeOf aMallocSizeOf) const
{
  // We don't know the answer since gfxDrawable doesn't expose this information.
  return 0;
}

void
DynamicImage::IncrementAnimationConsumers()
{ }

void
DynamicImage::DecrementAnimationConsumers()
{ }

#ifdef DEBUG
uint32_t
DynamicImage::GetAnimationConsumers()
{
  return 0;
}
#endif

nsresult
DynamicImage::OnImageDataAvailable(nsIRequest* aRequest,
                                   nsISupports* aContext,
                                   nsIInputStream* aInStr,
                                   uint64_t aSourceOffset,
                                   uint32_t aCount)
{
  return NS_OK;
}

nsresult
DynamicImage::OnImageDataComplete(nsIRequest* aRequest,
                                  nsISupports* aContext,
                                  nsresult aStatus,
                                  bool aLastPart)
{
  return NS_OK;
}

void
DynamicImage::OnSurfaceDiscarded()
{ }

void
DynamicImage::SetInnerWindowID(uint64_t aInnerWindowId)
{ }

uint64_t
DynamicImage::InnerWindowID() const
{
  return 0;
}

bool
DynamicImage::HasError()
{
  return !mDrawable;
}

void
DynamicImage::SetHasError()
{ }

ImageURL*
DynamicImage::GetURI()
{
  return nullptr;
}

// Methods inherited from XPCOM interfaces.

NS_IMPL_ISUPPORTS(DynamicImage, imgIContainer)

NS_IMETHODIMP
DynamicImage::GetWidth(int32_t* aWidth)
{
  *aWidth = mDrawable->Size().width;
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::GetHeight(int32_t* aHeight)
{
  *aHeight = mDrawable->Size().height;
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::GetIntrinsicSize(nsSize* aSize)
{
  gfxIntSize intSize(mDrawable->Size());
  *aSize = nsSize(intSize.width, intSize.height);
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::GetIntrinsicRatio(nsSize* aSize)
{
  gfxIntSize intSize(mDrawable->Size());
  *aSize = nsSize(intSize.width, intSize.height);
  return NS_OK;
}

NS_IMETHODIMP_(Orientation)
DynamicImage::GetOrientation()
{
  return Orientation();
}

NS_IMETHODIMP
DynamicImage::GetType(uint16_t* aType)
{
  *aType = imgIContainer::TYPE_RASTER;
  return NS_OK;
}

NS_IMETHODIMP_(uint16_t)
DynamicImage::GetType()
{
  return imgIContainer::TYPE_RASTER;
}

NS_IMETHODIMP
DynamicImage::GetAnimated(bool* aAnimated)
{
  *aAnimated = false;
  return NS_OK;
}

NS_IMETHODIMP_(TemporaryRef<SourceSurface>)
DynamicImage::GetFrame(uint32_t aWhichFrame,
                       uint32_t aFlags)
{
  gfxIntSize size(mDrawable->Size());

  RefPtr<DrawTarget> dt = gfxPlatform::GetPlatform()->
    CreateOffscreenContentDrawTarget(IntSize(size.width, size.height),
                                     SurfaceFormat::B8G8R8A8);
  nsRefPtr<gfxContext> context = new gfxContext(dt);

  nsresult rv = Draw(context, size, ImageRegion::Create(size),
                     aWhichFrame, GraphicsFilter::FILTER_NEAREST,
                     Nothing(), aFlags);

  NS_ENSURE_SUCCESS(rv, nullptr);
  return dt->Snapshot();
}

NS_IMETHODIMP_(bool)
DynamicImage::IsOpaque()
{
  // XXX(seth): For performance reasons it'd be better to return true here, but
  // I'm not sure how we can guarantee it for an arbitrary gfxDrawable.
  return false;
}

NS_IMETHODIMP
DynamicImage::GetImageContainer(LayerManager* aManager, ImageContainer** _retval)
{
  *_retval = nullptr;
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::Draw(gfxContext* aContext,
                   const nsIntSize& aSize,
                   const ImageRegion& aRegion,
                   uint32_t aWhichFrame,
                   GraphicsFilter aFilter,
                   const Maybe<SVGImageContext>& aSVGContext,
                   uint32_t aFlags)
{
  MOZ_ASSERT(!aSize.IsEmpty(), "Unexpected empty size");

  gfxIntSize drawableSize(mDrawable->Size());

  if (aSize == drawableSize) {
    gfxUtils::DrawPixelSnapped(aContext, mDrawable, drawableSize, aRegion,
                               SurfaceFormat::B8G8R8A8, aFilter);
    return NS_OK;
  }

  gfxSize scale(double(aSize.width) / drawableSize.width,
                double(aSize.height) / drawableSize.height);

  ImageRegion region(aRegion);
  region.Scale(1.0 / scale.width, 1.0 / scale.height);

  gfxContextMatrixAutoSaveRestore saveMatrix(aContext);
  aContext->Multiply(gfxMatrix::Scaling(scale.width, scale.height));

  gfxUtils::DrawPixelSnapped(aContext, mDrawable, drawableSize, region,
                             SurfaceFormat::B8G8R8A8, aFilter);
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::RequestDecode()
{
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::StartDecoding()
{
  return NS_OK;
}

bool
DynamicImage::IsDecoded()
{
  return true;
}

NS_IMETHODIMP
DynamicImage::LockImage()
{
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::UnlockImage()
{
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::RequestDiscard()
{
  return NS_OK;
}

NS_IMETHODIMP_(void)
DynamicImage::RequestRefresh(const mozilla::TimeStamp& aTime)
{ }

NS_IMETHODIMP
DynamicImage::GetAnimationMode(uint16_t* aAnimationMode)
{
  *aAnimationMode = kNormalAnimMode;
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::SetAnimationMode(uint16_t aAnimationMode)
{
  return NS_OK;
}

NS_IMETHODIMP
DynamicImage::ResetAnimation()
{
  return NS_OK;
}

NS_IMETHODIMP_(float)
DynamicImage::GetFrameIndex(uint32_t aWhichFrame)
{
  return 0;
}

NS_IMETHODIMP_(int32_t)
DynamicImage::GetFirstFrameDelay()
{
  return 0;
}

NS_IMETHODIMP_(void)
DynamicImage::SetAnimationStartTime(const mozilla::TimeStamp& aTime)
{ }

nsIntSize
DynamicImage::OptimalImageSizeForDest(const gfxSize& aDest, uint32_t aWhichFrame, GraphicsFilter aFilter, uint32_t aFlags)
{
  gfxIntSize size(mDrawable->Size());
  return nsIntSize(size.width, size.height);
}

NS_IMETHODIMP_(nsIntRect)
DynamicImage::GetImageSpaceInvalidationRect(const nsIntRect& aRect)
{
  return aRect;
}

already_AddRefed<imgIContainer>
DynamicImage::Unwrap()
{
  nsCOMPtr<imgIContainer> self(this);
  return self.forget();
}

} // namespace image
} // namespace mozilla