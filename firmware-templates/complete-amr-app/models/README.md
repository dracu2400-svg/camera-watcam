# Trained Model Directory

This directory should contain your trained digit recognition model as a C header file.

## Required File

**Filename:** `water_meter_digit_model.h`

This file should be generated using the model export tools (see `tools/training/generate_model_header.py`).

## Expected Format

The header file should contain:

```c
/**
 * Auto-generated TensorFlow Lite model
 */

#ifndef WATER_METER_DIGIT_MODEL_H
#define WATER_METER_DIGIT_MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

// Model size
const unsigned int digit_recognition_model_len = 24832;

// Model data (INT8 quantized TFLite model)
const unsigned char digit_recognition_model[] = {
  0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33,
  // ... (rest of model data)
};

#ifdef __cplusplus
}
#endif

#endif // WATER_METER_DIGIT_MODEL_H
```

## How to Generate This File

Follow the complete workflow guide:

1. **Train your model** using the training GUI (`tools/training-gui/`)
2. **Export to TFLite** (INT8 quantized)
3. **Generate C header** using the conversion tool:

```bash
cd tools/training
python3 generate_model_header.py \
    --input ../training-gui/data/models/digit_model.tflite \
    --output ../../models/water_meter_digit_model.h \
    --array-name digit_recognition_model
```

4. **Copy to firmware template**:

```bash
cp models/water_meter_digit_model.h \
   firmware-templates/complete-amr-app/models/
```

## Verification

To verify your model header is correct:

1. Check it defines both:
   - `digit_recognition_model[]` (const unsigned char array)
   - `digit_recognition_model_len` (const unsigned int)

2. Verify the model size is reasonable (~20-30 KB for typical digit recognition)

3. Test compilation:
   ```bash
   grep "digit_recognition_model_len" water_meter_digit_model.h
   # Should output something like: const unsigned int digit_recognition_model_len = 24832;
   ```

## For Testing/Development

If you don't have a trained model yet, you can create a placeholder for compilation testing:

```c
// PLACEHOLDER - Replace with actual trained model!
#ifndef WATER_METER_DIGIT_MODEL_H
#define WATER_METER_DIGIT_MODEL_H
const unsigned int digit_recognition_model_len = 100;
const unsigned char digit_recognition_model[] = {
    0x1c, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33,
    // ... minimal placeholder data
};
#endif
```

**WARNING:** Placeholder models will not provide meaningful digit recognition results!

## References

- Full training guide: `docs/10-step-by-step-guide/README.md`
- Training GUI: `tools/training-gui/`
- Model conversion: `tools/training/generate_model_header.py`
