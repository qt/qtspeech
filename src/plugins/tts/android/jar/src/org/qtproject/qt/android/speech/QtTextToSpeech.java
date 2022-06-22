// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only

package org.qtproject.qt.android.speech;

import android.content.ContentResolver;
import android.content.Context;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.Engine;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.speech.tts.UtteranceProgressListener;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import java.lang.Float;
import java.util.HashMap;
import java.util.Locale;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;

public class QtTextToSpeech
{
    // Native callback functions
    native public void notifyError(long id, long reason);
    native public void notifyReady(long id);
    native public void notifySpeaking(long id);

    private TextToSpeech mTts;
    private final long mId;
    private boolean mInitialized = false;
    private float mPitch = 1.0f;
    private float mRate = 1.0f;
    private float mVolume = 1.0f;

    private String TAG = "QtTextToSpeech";
    // OnInitListener
    private final OnInitListener mTtsChangeListener = new OnInitListener() {
        @Override
        public void onInit(int status) {
            if (status == TextToSpeech.SUCCESS) {
                mInitialized = true;
                notifyReady(mId);
                Log.d(TAG, "TTS initialized");
            } else {
                mInitialized = false;
                notifyError(mId, 1); // QTextToSpeech::ErrorReason::Initialization
                Log.w(TAG, "TTS initialization failed");
            }
        }
    };

    private String utteranceTAG = "UtteranceProgressListener";
    // UtteranceProgressListener
    private final UtteranceProgressListener mTtsUtteranceProgressListener = new UtteranceProgressListener() {
        @Override
        public void onDone(String utteranceId) {
            Log.d(utteranceTAG, "onDone");
            if (utteranceId.equals("UtteranceId")) {
                notifyReady(mId);
            }
        }

        @Override
        public void onError(String utteranceId) {
            Log.w(utteranceTAG, "onError");
            if (utteranceId.equals("UtteranceId")) {
                notifyError(mId, 4);  // QTextToSpeech::ErrorReason::Playback
            }
        }

        @Override
        public void onStart(String utteranceId) {
            Log.d(utteranceTAG, "onStart");
            if (utteranceId.equals("UtteranceId")) {
                notifySpeaking(mId);
            }
        }
    };

    QtTextToSpeech(final Context context, final long id, String engine) {
        mId = id;
        if (engine.isEmpty()) {
            mTts = new TextToSpeech(context, mTtsChangeListener);
        } else {
            mTts = new TextToSpeech(context, mTtsChangeListener, engine);
        }
        mTts.setOnUtteranceProgressListener(mTtsUtteranceProgressListener);

        // Read pitch from settings
        ContentResolver resolver = context.getContentResolver();
        try {
            float pitch = Settings.Secure.getFloat(resolver, android.provider.Settings.Secure.TTS_DEFAULT_PITCH);
            mPitch = pitch / 100.0f;
        } catch (SettingNotFoundException e) {
            mPitch = 1.0f;
        }

        // Read rate from settings
        try {
            float rate = Settings.Secure.getFloat(resolver, android.provider.Settings.Secure.TTS_DEFAULT_RATE);
            mRate = rate / 100.0f;
        } catch (SettingNotFoundException e) {
            mRate = 1.0f;
        }
    }

    public void say(String text)
    {
        Log.d(TAG, "TTS say(): " + text);
        int result = -1;

        HashMap<String, String> map = new HashMap<String, String>();
        map.put(TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID, "UtteranceId");
        map.put(TextToSpeech.Engine.KEY_PARAM_VOLUME, Float.toString(mVolume));
        result = mTts.speak(text, TextToSpeech.QUEUE_FLUSH, map);

        Log.d(TAG, "TTS say() result: " + Integer.toString(result));
        if (result == TextToSpeech.ERROR)
            notifyError(mId, 3); // QTextToSpeech::ErrorReason::Input
    }

    public void stop()
    {
        Log.d(TAG, "Stopping TTS");
        mTts.stop();
    }

    public float pitch()
    {
        return mPitch;
    }

    public int setPitch(float pitch)
    {
        if (Float.compare(pitch, mPitch) == 0)
            return TextToSpeech.ERROR;

        int success = mTts.setPitch(pitch);
        if (success == TextToSpeech.SUCCESS)
            mPitch = pitch;
        else
            notifyError(mId, 2); // QTextToSpeech::ErrorReason::Configuration

        return success;
    }

    public float rate()
    {
        return mRate;
    }

    public int setRate(float rate)
    {
        if (Float.compare(rate, mRate) == 0)
            return TextToSpeech.ERROR;

        int success = mTts.setSpeechRate(rate);
        if (success == TextToSpeech.SUCCESS)
            mRate = rate;
        else
            notifyError(mId, 2); // QTextToSpeech::ErrorReason::Configuration

        return success;
    }

    public void shutdown()
    {
        mTts.shutdown();
    }

    public float volume()
    {
        return mVolume;
    }

    public int setVolume(float volume)
    {
        if (Float.compare(volume, mVolume) == 0)
            return TextToSpeech.ERROR;

        mVolume = volume;
        return TextToSpeech.SUCCESS;
    }

    public boolean setLocale(Locale locale)
    {
        int result = mTts.setLanguage(locale);
        return (result != TextToSpeech.LANG_NOT_SUPPORTED) && (result != TextToSpeech.LANG_MISSING_DATA);
    }

    public List<Object> getAvailableVoices()
    {
        if (mInitialized && android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            //Log.d(TAG, "Voices: " + mTts.getVoices());
            return new ArrayList<Object>(mTts.getVoices());
        }
        return new ArrayList<Object>();
    }

    public List<Locale> getAvailableLocales()
    {
        if (mInitialized && android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            //Log.d(TAG, "Locales: " + mTts.getAvailableLanguages());
            final Set<Locale> languages = mTts.getAvailableLanguages();
            ArrayList<Locale> locales = new ArrayList<Locale>();

            for (Locale language : languages) {
                String languageCode = language.getLanguage();
                String countryCode = language.getCountry();

                if (languageCode.equals(language.getISO3Language()))
                    languageCode = convertLanguageCodeThreeDigitToTwoDigit(languageCode);
                if (countryCode.equals(language.getISO3Country()))
                    countryCode = convertCountryCodeThreeDigitToTwoDigit(countryCode);

                locales.add(new Locale(languageCode, countryCode));
            }

            return locales;
        }
        return new ArrayList<Locale>();
    }

    public Locale getLocale()
    {
        //Log.d(TAG, "getLocale: " + mLocale);
        final Locale language = mTts.getLanguage();
        String languageCode = language.getLanguage();
        String countryCode = language.getCountry();

        if (languageCode.equals(language.getISO3Language()))
            languageCode = convertLanguageCodeThreeDigitToTwoDigit(languageCode);
        if (countryCode.equals(language.getISO3Country()))
            countryCode = convertCountryCodeThreeDigitToTwoDigit(countryCode);

        return new Locale(languageCode, countryCode);
    }

    public Object getVoice()
    {
        if (mInitialized && android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            return mTts.getVoice();
        }
        return null;
    }

    public boolean setVoice(String voiceName)
    {
        if (!mInitialized)
            return false;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
             for (android.speech.tts.Voice voice : mTts.getVoices()) {
                 if (voice.getName().equals(voiceName)) {
                     int result = mTts.setVoice(voice);
                     if (result == TextToSpeech.SUCCESS) {
                         //Log.d(TAG, "setVoice: " + voice);
                         return true;
                     }
                     break;
                 }
             }
        }
        return false;
    }

    private String convertLanguageCodeThreeDigitToTwoDigit(String iso3Language)
    {
        final String[] isoLanguages = Locale.getISOLanguages();

        for (String isoLanguage : isoLanguages) {
            if (iso3Language.equals(new Locale(isoLanguage).getISO3Language())) {
                return isoLanguage;
            }
        }

        return iso3Language;
    }

    private String convertCountryCodeThreeDigitToTwoDigit(String iso3Country)
    {
        final String[] isoCountries = Locale.getISOCountries();

        for (String isoCountry : isoCountries) {
            if (iso3Country.equals(new Locale("en", isoCountry).getISO3Country())) {
                return isoCountry;
            }
        }

        return iso3Country;
    }
}
