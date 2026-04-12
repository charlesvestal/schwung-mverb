#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include "audio_fx_api_v2.h"
#include "MVerb.h"

typedef struct {
    MVerb<float> reverb;
    float damping;
    float density;
    float bandwidth;
    float decay;
    float predelay;
    float size;
    float gain;
    float mix;
    float early_mix;
} mverb_instance_t;

static const host_api_v1_t *g_host = NULL;
static audio_fx_api_v2_t g_api;

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void log_msg(const char *msg) {
    if (g_host && g_host->log) {
        char buf[256];
        snprintf(buf, sizeof(buf), "[mverb] %s", msg);
        g_host->log(buf);
    }
}

static int json_get_float(const char *json, const char *key, float *out) {
    char search[64];
    const char *p;
    snprintf(search, sizeof(search), "\"%s\":", key);
    p = strstr(json, search);
    if (!p) return -1;
    p += strlen(search);
    while (*p == ' ' || *p == '\t') p++;
    *out = (float)atof(p);
    return 0;
}

static void apply_params(mverb_instance_t *inst) {
    inst->reverb.setParameter(MVerb<float>::DAMPINGFREQ, inst->damping);
    inst->reverb.setParameter(MVerb<float>::DENSITY, inst->density);
    inst->reverb.setParameter(MVerb<float>::BANDWIDTHFREQ, inst->bandwidth);
    inst->reverb.setParameter(MVerb<float>::DECAY, inst->decay);
    inst->reverb.setParameter(MVerb<float>::PREDELAY, inst->predelay);
    inst->reverb.setParameter(MVerb<float>::SIZE, inst->size);
    inst->reverb.setParameter(MVerb<float>::GAIN, inst->gain);
    inst->reverb.setParameter(MVerb<float>::MIX, inst->mix);
    inst->reverb.setParameter(MVerb<float>::EARLYMIX, inst->early_mix);
}

static void *create_instance(const char *module_dir, const char *config_json) {
    mverb_instance_t *inst;
    float v;
    (void)module_dir;

    inst = new mverb_instance_t();
    if (!inst) return NULL;

    inst->damping = 0.5f;
    inst->density = 0.5f;
    inst->bandwidth = 0.75f;
    inst->decay = 0.5f;
    inst->predelay = 0.0f;
    inst->size = 0.5f;
    inst->gain = 1.0f;
    inst->mix = 0.35f;
    inst->early_mix = 0.75f;

    inst->reverb.setSampleRate((float)(g_host ? g_host->sample_rate : 44100));
    if (config_json) {
        if (json_get_float(config_json, "mix", &v) == 0) inst->mix = clampf(v, 0.0f, 1.0f);
    }
    apply_params(inst);
    return inst;
}

static void destroy_instance(void *instance) {
    delete (mverb_instance_t *)instance;
}

static void set_param(void *instance, const char *key, const char *val) {
    mverb_instance_t *inst = (mverb_instance_t *)instance;
    float v;
    if (!inst || !key || !val) return;

    if (strcmp(key, "state") == 0) {
        if (json_get_float(val, "damping", &v) == 0) inst->damping = clampf(v, 0.0f, 1.0f);
        if (json_get_float(val, "density", &v) == 0) inst->density = clampf(v, 0.0f, 1.0f);
        if (json_get_float(val, "bandwidth", &v) == 0) inst->bandwidth = clampf(v, 0.0f, 1.0f);
        if (json_get_float(val, "decay", &v) == 0) inst->decay = clampf(v, 0.0f, 1.0f);
        if (json_get_float(val, "predelay", &v) == 0) inst->predelay = clampf(v, 0.0f, 1.0f);
        if (json_get_float(val, "size", &v) == 0) inst->size = clampf(v, 0.0f, 1.0f);
        if (json_get_float(val, "gain", &v) == 0) inst->gain = clampf(v, 0.0f, 1.5f);
        if (json_get_float(val, "mix", &v) == 0) inst->mix = clampf(v, 0.0f, 1.0f);
        if (json_get_float(val, "early_mix", &v) == 0) inst->early_mix = clampf(v, 0.0f, 1.0f);
        apply_params(inst);
        return;
    }

    v = (float)atof(val);
    if (strcmp(key, "damping") == 0) inst->damping = clampf(v, 0.0f, 1.0f);
    else if (strcmp(key, "density") == 0) inst->density = clampf(v, 0.0f, 1.0f);
    else if (strcmp(key, "bandwidth") == 0) inst->bandwidth = clampf(v, 0.0f, 1.0f);
    else if (strcmp(key, "decay") == 0) inst->decay = clampf(v, 0.0f, 1.0f);
    else if (strcmp(key, "predelay") == 0) inst->predelay = clampf(v, 0.0f, 1.0f);
    else if (strcmp(key, "size") == 0) inst->size = clampf(v, 0.0f, 1.0f);
    else if (strcmp(key, "gain") == 0) inst->gain = clampf(v, 0.0f, 1.5f);
    else if (strcmp(key, "mix") == 0) inst->mix = clampf(v, 0.0f, 1.0f);
    else if (strcmp(key, "early_mix") == 0) inst->early_mix = clampf(v, 0.0f, 1.0f);

    apply_params(inst);
}

static int get_param(void *instance, const char *key, char *buf, int buf_len) {
    mverb_instance_t *inst = (mverb_instance_t *)instance;
    if (!inst) return -1;

    if (strcmp(key, "damping") == 0) return snprintf(buf, buf_len, "%.3f", inst->damping);
    if (strcmp(key, "density") == 0) return snprintf(buf, buf_len, "%.3f", inst->density);
    if (strcmp(key, "bandwidth") == 0) return snprintf(buf, buf_len, "%.3f", inst->bandwidth);
    if (strcmp(key, "decay") == 0) return snprintf(buf, buf_len, "%.3f", inst->decay);
    if (strcmp(key, "predelay") == 0) return snprintf(buf, buf_len, "%.3f", inst->predelay);
    if (strcmp(key, "size") == 0) return snprintf(buf, buf_len, "%.3f", inst->size);
    if (strcmp(key, "gain") == 0) return snprintf(buf, buf_len, "%.3f", inst->gain);
    if (strcmp(key, "mix") == 0) return snprintf(buf, buf_len, "%.3f", inst->mix);
    if (strcmp(key, "early_mix") == 0) return snprintf(buf, buf_len, "%.3f", inst->early_mix);
    if (strcmp(key, "name") == 0) return snprintf(buf, buf_len, "MVerb");
    if (strcmp(key, "state") == 0) {
        return snprintf(
            buf, buf_len,
            "{\"damping\":%.4f,\"density\":%.4f,\"bandwidth\":%.4f,\"decay\":%.4f,\"predelay\":%.4f,\"size\":%.4f,\"gain\":%.4f,\"mix\":%.4f,\"early_mix\":%.4f}",
            inst->damping, inst->density, inst->bandwidth, inst->decay,
            inst->predelay, inst->size, inst->gain, inst->mix, inst->early_mix
        );
    }
    if (strcmp(key, "ui_hierarchy") == 0) {
        const char *json =
            "{"
            "\"modes\":null,"
            "\"levels\":{"
            "\"root\":{"
            "\"knobs\":[\"damping\",\"density\",\"bandwidth\",\"decay\",\"predelay\",\"size\",\"gain\",\"mix\"],"
            "\"params\":[\"damping\",\"density\",\"bandwidth\",\"decay\",\"predelay\",\"size\",\"gain\",\"mix\",\"early_mix\"]"
            "}"
            "}"
            "}";
        int len = (int)strlen(json);
        if (len >= buf_len) return -1;
        strcpy(buf, json);
        return len;
    }
    if (strcmp(key, "chain_params") == 0) {
        const char *json =
            "["
            "{\"key\":\"damping\",\"name\":\"Damping\",\"type\":\"float\",\"min\":0,\"max\":1,\"default\":0.5,\"step\":0.01},"
            "{\"key\":\"density\",\"name\":\"Density\",\"type\":\"float\",\"min\":0,\"max\":1,\"default\":0.5,\"step\":0.01},"
            "{\"key\":\"bandwidth\",\"name\":\"Bandwidth\",\"type\":\"float\",\"min\":0,\"max\":1,\"default\":0.75,\"step\":0.01},"
            "{\"key\":\"decay\",\"name\":\"Decay\",\"type\":\"float\",\"min\":0,\"max\":1,\"default\":0.5,\"step\":0.01},"
            "{\"key\":\"predelay\",\"name\":\"Predelay\",\"type\":\"float\",\"min\":0,\"max\":1,\"default\":0,\"step\":0.01},"
            "{\"key\":\"size\",\"name\":\"Size\",\"type\":\"float\",\"min\":0,\"max\":1,\"default\":0.5,\"step\":0.01},"
            "{\"key\":\"gain\",\"name\":\"Gain\",\"type\":\"float\",\"min\":0,\"max\":1.5,\"default\":1,\"step\":0.01},"
            "{\"key\":\"mix\",\"name\":\"Mix\",\"type\":\"float\",\"min\":0,\"max\":1,\"default\":0.35,\"step\":0.01},"
            "{\"key\":\"early_mix\",\"name\":\"Early Mix\",\"type\":\"float\",\"min\":0,\"max\":1,\"default\":0.75,\"step\":0.01}"
            "]";
        int len = (int)strlen(json);
        if (len >= buf_len) return -1;
        strcpy(buf, json);
        return len;
    }

    return -1;
}

static void process_block(void *instance, int16_t *audio_inout, int frames) {
    mverb_instance_t *inst = (mverb_instance_t *)instance;
    float in_l[128];
    float in_r[128];
    float *inputs[2] = { in_l, in_r };
    float out_l[128];
    float out_r[128];
    float *outputs[2] = { out_l, out_r };
    int i;

    if (!inst || !audio_inout) return;
    if (frames > 128) frames = 128;

    for (i = 0; i < frames; ++i) {
        in_l[i] = audio_inout[i * 2] / 32768.0f;
        in_r[i] = audio_inout[i * 2 + 1] / 32768.0f;
    }

    inst->reverb.process(inputs, outputs, frames);

    for (i = 0; i < frames; ++i) {
        float l = clampf(out_l[i], -1.0f, 1.0f);
        float r = clampf(out_r[i], -1.0f, 1.0f);
        audio_inout[i * 2] = (int16_t)(l * 32767.0f);
        audio_inout[i * 2 + 1] = (int16_t)(r * 32767.0f);
    }
}

extern "C" audio_fx_api_v2_t *move_audio_fx_init_v2(const host_api_v1_t *host) {
    g_host = host;
    memset(&g_api, 0, sizeof(g_api));
    g_api.api_version = AUDIO_FX_API_VERSION_2;
    g_api.create_instance = create_instance;
    g_api.destroy_instance = destroy_instance;
    g_api.process_block = process_block;
    g_api.set_param = set_param;
    g_api.get_param = get_param;
    g_api.on_midi = NULL;
    log_msg("initialized");
    return &g_api;
}
