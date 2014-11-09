/*
  ==============================================================================

   This file contains code which forms part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================

  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014 Andrew M Taylor <a.m.taylor303@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "shurikensampler.h"
//#include <QDebug>


ShurikenSamplerSound::ShurikenSamplerSound( const SharedSampleBuffer sampleBuffer,
                                            const qreal sampleRate,
                                            const BigInteger& notes,
                                            const int midiNoteForNormalPitch ) :
    mData( sampleBuffer ),
    mOriginalStartFrame( 0 ),
    mOriginalEndFrame( sampleBuffer->getNumFrames() - 1 ),
    mSourceSampleRate( sampleRate ),
    mMidiNotes( notes ),
    mMidiRootNote( midiNoteForNormalPitch )
{
    mAttackSamples = 0;
    mReleaseSamples = 0;

    mStartFrame = mOriginalStartFrame;
    mEndFrame = mOriginalEndFrame;

    mTempStartFrame = mOriginalStartFrame;
    mTempEndFrame = mOriginalEndFrame;

    mIsTempSampleRangeSet = false;
}



ShurikenSamplerSound::~ShurikenSamplerSound()
{
}



void ShurikenSamplerSound::setTempSampleRange( const SharedSampleRange sampleRange )
{
    mTempStartFrame = sampleRange->startFrame;
    mTempEndFrame = sampleRange->startFrame + sampleRange->numFrames - 1;

    mIsTempSampleRangeSet = true;
}



bool ShurikenSamplerSound::appliesToNote( const int midiNoteNumber )
{
    return mMidiNotes[ midiNoteNumber ];
}



bool ShurikenSamplerSound::appliesToChannel( const int /*midiChannel*/ )
{
    return true;
}



//==============================================================================
ShurikenSamplerVoice::ShurikenSamplerVoice()
    : mPitchRatio( 0.0 ),
      mSourceSamplePosition( 0.0 ),
      mLeftGain( 0.0f ), mRightGain( 0.0f ),
      mAttackReleaseLevel( 0 ), mAttackDelta( 0 ), mReleaseDelta( 0 ),
      mIsInAttack( false ), mIsInRelease( false )
{
}



ShurikenSamplerVoice::~ShurikenSamplerVoice()
{
}



bool ShurikenSamplerVoice::canPlaySound( SynthesiserSound* sound )
{
    return dynamic_cast<const ShurikenSamplerSound*>( sound ) != nullptr;
}



void ShurikenSamplerVoice::startNote( const int midiNoteNumber,
                                      const float velocity,
                                      SynthesiserSound* s,
                                      const int /*currentPitchWheelPosition*/ )
{
    if ( ShurikenSamplerSound* const sound = dynamic_cast<ShurikenSamplerSound*>( s ) )
    {
        mPitchRatio = pow( 2.0, (midiNoteNumber - sound->mMidiRootNote) / 12.0 )
                        * sound->mSourceSampleRate / getSampleRate();

        if ( sound->mIsTempSampleRangeSet )
        {
            sound->mStartFrame = sound->mTempStartFrame;
            sound->mEndFrame = sound->mTempEndFrame;
            sound->mIsTempSampleRangeSet = false;
        }

        mSourceSamplePosition = sound->mStartFrame;
        mLeftGain = velocity;
        mRightGain = velocity;

        mIsInAttack =( sound->mAttackSamples > 0 );
        mIsInRelease = false;

        if ( mIsInAttack )
        {
            mAttackReleaseLevel = 0.0f;
            mAttackDelta = (float) (mPitchRatio / sound->mAttackSamples);
        }
        else
        {
            mAttackReleaseLevel = 1.0f;
            mAttackDelta = 0.0f;
        }

        if ( sound->mReleaseSamples > 0 )
            mReleaseDelta = (float) ( -mPitchRatio / sound->mReleaseSamples );
        else
            mReleaseDelta = 0.0f;
    }
    else
    {
        jassertfalse; // this object can only play ShurikenSamplerSounds!
    }
}



void ShurikenSamplerVoice::stopNote( const float /*velocity*/, const bool allowTailOff )
{
    ShurikenSamplerSound* const playingSound =
             dynamic_cast<ShurikenSamplerSound*>( getCurrentlyPlayingSound().get() );

    if ( allowTailOff )
    {
        mIsInAttack = false;
        mIsInRelease = true;
    }
    else
    {
        clearCurrentNote();
    }

    if ( playingSound != NULL )
    {
        playingSound->mStartFrame = playingSound->mOriginalStartFrame;
        playingSound->mEndFrame = playingSound->mOriginalEndFrame;
    }
}



void ShurikenSamplerVoice::pitchWheelMoved( const int /*newValue*/ )
{
}



void ShurikenSamplerVoice::controllerMoved( const int /*controllerNumber*/,
                                            const int /*newValue*/ )
{
}



//==============================================================================
void ShurikenSamplerVoice::renderNextBlock( AudioSampleBuffer& outputBuffer, int startSample, int numSamples )
{
    if ( const ShurikenSamplerSound* const playingSound =
         static_cast<ShurikenSamplerSound*>( getCurrentlyPlayingSound().get() ) )
    {
        const float* const inL = playingSound->mData->getReadPointer( 0 );
        const float* const inR = playingSound->mData->getNumChannels() > 1
                                    ? playingSound->mData->getReadPointer( 1 ) : nullptr;

        float* outL = outputBuffer.getWritePointer( 0, startSample );
        float* outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer( 1, startSample ) : nullptr;

        while ( --numSamples >= 0 )
        {
            const int pos = (int) mSourceSamplePosition;
            const float alpha = (float) ( mSourceSamplePosition - pos );
            const float invAlpha = 1.0f - alpha;

            // just using a very simple linear interpolation here..
            float l = ( inL [pos] * invAlpha + inL [pos + 1] * alpha );
            float r = ( inR != nullptr ) ? ( inR [pos] * invAlpha + inR [pos + 1] * alpha ) : l;

            l *= mLeftGain;
            r *= mRightGain;

            if ( mIsInAttack )
            {
                l *= mAttackReleaseLevel;
                r *= mAttackReleaseLevel;

                mAttackReleaseLevel += mAttackDelta;

                if ( mAttackReleaseLevel >= 1.0f )
                {
                    mAttackReleaseLevel = 1.0f;
                    mIsInAttack = false;
                }
            }
            else if ( mIsInRelease )
            {
                l *= mAttackReleaseLevel;
                r *= mAttackReleaseLevel;

                mAttackReleaseLevel += mReleaseDelta;

                if ( mAttackReleaseLevel <= 0.0f )
                {
                    stopNote( 0.0f, false );
                    break;
                }
            }

            if ( outR != nullptr )
            {
                *outL++ += l;
                *outR++ += r;
            }
            else
            {
                *outL++ += (l + r) * 0.5f;
            }

            mSourceSamplePosition += mPitchRatio;

            if ( mSourceSamplePosition > playingSound->mEndFrame )
            {
                stopNote( 0.0f, false );
                break;
            }
        }
    }
}
