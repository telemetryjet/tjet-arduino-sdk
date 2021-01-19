/*
TelemetryJet Arduino SDK
Chris Dalke <chrisdalke@gmail.com>

Lightweight communication library for hardware telemetry data. 
Handles bidirectional communication and state management for data points. 
-------------------------------------------------------------------------
Part of the TelemetryJet platform -- Collect, analyze, and share
data from your hardware. Code not required.

Distributed "as is" under the MIT License. See LICENSE.md for details.
*/

#include "TelemetryJet.h"
#include "MessagePack.h"

const char* timestampField = "ts";

TelemetryJet::TelemetryJet(Stream *transport, unsigned long transmitRate)
  : transport(transport), transmitRate(transmitRate) {
  // Initialize variable-size dimensions array
  dimensions = (DataPoint**) malloc(sizeof(DataPoint*) * dimensionCacheLength);
}

void TelemetryJet::update() {
  if (isTextMode) {
    // Text mode
    // Don't read inputs; just log as text output to the serial stream
    // Useful for debugging purposes
    while (transport->available() > 0) {
      uint8_t inByte = transport->read();
    }

    if (millis() - lastSent >= transmitRate && numDimensions > 0) {
      for (uint16_t i = 0; i < numDimensions; i++) {
        if (dimensions[i]->hasValue && (dimensions[i]->hasNewValue || !isDeltaMode)) {
          dimensions[i]->hasNewValue = false;
          switch (dimensions[i]->type) {
            case DataPointType::BOOLEAN: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((unsigned int)(dimensions[i]->value.v_bool));
              transport->write('\n');
              break;
            }
            case DataPointType::UINT8: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((unsigned int)(dimensions[i]->value.v_uint8));
              transport->write('\n');
              break;
            }
            case DataPointType::UINT16: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((unsigned int)(dimensions[i]->value.v_uint16));
              transport->write('\n');
              break;
            }
            case DataPointType::UINT32: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((unsigned long)(dimensions[i]->value.v_uint32));
              transport->write('\n');
              break;
            }
            case DataPointType::UINT64: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((unsigned long)(dimensions[i]->value.v_uint64));
              transport->write('\n');
              break;
            }
            case DataPointType::INT8: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((int)(dimensions[i]->value.v_int8));
              transport->write('\n');
              break;
            }
            case DataPointType::INT16: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((int)(dimensions[i]->value.v_int16));
              transport->write('\n');
              break;
            }
            case DataPointType::INT32: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((long)(dimensions[i]->value.v_int32));
              transport->write('\n');
              break;
            }
            case DataPointType::INT64: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((long)(dimensions[i]->value.v_int64));
              transport->write('\n');
              break;
            }
            case DataPointType::FLOAT32: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((double)(dimensions[i]->value.v_float32));
              transport->write('\n');
              break;
            }
            case DataPointType::FLOAT64: {
              transport->print((unsigned int)dimensions[i]->key);
              transport->print('=');
              transport->print((double)(dimensions[i]->value.v_float64));
              transport->write('\n');
              break;
            }
            default: {
              break;
            }
          }
        }
      }
      lastSent = millis();
    }
  } else {
    // Binary mode
    // Full-featured input and output
    while (transport->available() > 0) {
      uint8_t inByte = transport->read();
    }
  }
}

Dimension TelemetryJet::createDimension(uint16_t key, uint32_t timeoutAge = 0) {
  // Resize dimension array if it is full
  if (numDimensions >= dimensionCacheLength) {
    DataPoint** newDimensionArray = (DataPoint**) malloc(sizeof(DataPoint*) * (dimensionCacheLength + 8));
    // Copy from old cache into new cache
    for (int i = 0; i < numDimensions; i++) {
      newDimensionArray[i] = dimensions[i];
    }
    free(dimensions);
    dimensions = newDimensionArray;
    dimensionCacheLength = dimensionCacheLength + 8;
  }

  uint16_t dimensionId = numDimensions++;
  // Create a data point for the new dimension and add to the cache array
  DataPoint* newDataPoint = (DataPoint*) malloc(sizeof(DataPoint));
  dimensions[dimensionId] = newDataPoint;
  dimensions[dimensionId]->key = key;
  dimensions[dimensionId]->type = DataPointType::FLOAT32;
  dimensions[dimensionId]->value.v_float32 = 0.0;
  dimensions[dimensionId]->hasValue = false;
  dimensions[dimensionId]->hasNewValue = false;
  if (timeoutAge > 0) {
    dimensions[dimensionId]->hasTimeout = true;
    dimensions[dimensionId]->timeoutInterval = timeoutAge;
  } else {
    dimensions[dimensionId]->hasTimeout = false;
    dimensions[dimensionId]->timeoutInterval = 0;
  }
  dimensions[dimensionId]->lastTimestamp = 0;
  return Dimension(dimensionId, this);
}

void Dimension::setBool(bool value) {
  _parent->dimensions[_id]->value.v_bool = value;
  _parent->dimensions[_id]->type = DataPointType::BOOLEAN;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setUInt8(uint8_t value) {
  _parent->dimensions[_id]->value.v_uint8 = value;
  _parent->dimensions[_id]->type = DataPointType::UINT8;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setUInt16(uint16_t value) {
  _parent->dimensions[_id]->value.v_uint16 = value;
  _parent->dimensions[_id]->type = DataPointType::UINT16;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setUInt32(uint32_t value) {
  
  _parent->dimensions[_id]->value.v_uint32 = value;
  _parent->dimensions[_id]->type = DataPointType::UINT32;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setUInt64(uint64_t value) {
  _parent->dimensions[_id]->value.v_uint64 = value;
  _parent->dimensions[_id]->type = DataPointType::UINT64;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setInt8(int8_t value) {
  _parent->dimensions[_id]->value.v_int8 = value;
  _parent->dimensions[_id]->type = DataPointType::INT8;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setInt16(int16_t value) {
  _parent->dimensions[_id]->value.v_int16 = value;
  _parent->dimensions[_id]->type = DataPointType::INT16;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setInt32(int32_t value) {
  _parent->dimensions[_id]->value.v_int32 = value;
  _parent->dimensions[_id]->type = DataPointType::INT32;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setInt64(int64_t value) {
  _parent->dimensions[_id]->value.v_int64 = value;
  _parent->dimensions[_id]->type = DataPointType::INT64;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setFloat32(float value) {
  _parent->dimensions[_id]->value.v_float32 = value;
  _parent->dimensions[_id]->type = DataPointType::FLOAT32;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}

void Dimension::setFloat64(double value) {
  _parent->dimensions[_id]->value.v_float64 = value;
  _parent->dimensions[_id]->type = DataPointType::FLOAT64;
  _parent->dimensions[_id]->hasValue = true;
  _parent->dimensions[_id]->hasNewValue = true;
  _parent->dimensions[_id]->lastTimestamp = millis();
}


bool Dimension::getBool(bool defaultValue = false) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::BOOLEAN) {
    return _parent->dimensions[_id]->value.v_bool;
  } else {
    return defaultValue;
  }
}

uint8_t Dimension::getUInt8(uint8_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::UINT8) {
    return _parent->dimensions[_id]->value.v_uint8;
  } else {
    return (uint8_t)getBool(defaultValue);
  }
}

uint16_t Dimension::getUInt16(uint16_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::UINT16) {
    return _parent->dimensions[_id]->value.v_uint16;
  } else {
    return (uint16_t)getUInt8(defaultValue);
  }
}

uint32_t Dimension::getUInt32(uint32_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::UINT32) {
    return _parent->dimensions[_id]->value.v_uint32;
  } else {
    return (uint32_t)getUInt16(defaultValue);
  }
}

uint64_t Dimension::getUInt64(uint64_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::UINT64) {
    return _parent->dimensions[_id]->value.v_uint64;
  } else {
    return (uint64_t)getUInt32(defaultValue);
  }
}

int8_t Dimension::getInt8(int8_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::INT8) {
    return _parent->dimensions[_id]->value.v_int8;
  } else {
    return defaultValue;
  }
}

int16_t Dimension::getInt16(int16_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::INT16) {
    return _parent->dimensions[_id]->value.v_int16;
  } else {
    return (int16_t)getInt8(defaultValue);
  }
}

int32_t Dimension::getInt32(int32_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::INT32) {
    return _parent->dimensions[_id]->value.v_int32;
  } else {
    return (int32_t)getInt16(defaultValue);
  }
}

int64_t Dimension::getInt64(int64_t defaultValue = 0) {
  if (!hasValue()) {
    return defaultValue;
  }
  
  if (_parent->dimensions[_id]->type == DataPointType::INT64) {
    return _parent->dimensions[_id]->value.v_int64;
  } else {
    return (int64_t)getInt32(defaultValue);
  }
}

float Dimension::getFloat32(float defaultValue = 0.0) {
  if (!hasValue()) {
    return defaultValue;
  }

  if (_parent->dimensions[_id]->type == DataPointType::FLOAT32) {
    return _parent->dimensions[_id]->value.v_float32;
  } else {
    return defaultValue;
  }
}

double Dimension::getFloat64(double defaultValue = 0.0) {
  if (!hasValue()) {
    return defaultValue;
  }

  if (_parent->dimensions[_id]->type == DataPointType::FLOAT64) {
    return _parent->dimensions[_id]->value.v_float64;
  } else {
    return (double)getFloat32(defaultValue);
  }
}

bool Dimension::hasBool(bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::BOOLEAN) {
    return true;
  }
  if (!exact) {
    return false;
  }
}

bool  Dimension::hasUInt8  (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::UINT8) {
    return true;
  }
  if (!exact) {
    return hasBool();
  }
}

bool Dimension::hasUInt16 (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::UINT16) {
    return true;
  }
  if (!exact) {
    return hasUInt8();
  }
}

bool Dimension::hasUInt32 (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::UINT32) {
    return true;
  }
  if (!exact) {
    return hasUInt16();
  }
}

bool Dimension::hasUInt64 (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::UINT64) {
    return true;
  }
  if (!exact) {
    return hasUInt32();
  }
}

bool Dimension::hasInt8   (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::INT8) {
    return true;
  }
  if (!exact) {
    return false;
  }
}

bool Dimension::hasInt16  (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::INT16) {
    return true;
  }
  if (!exact) {
    return hasInt8();
  }
}

bool Dimension::hasInt32  (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::INT32) {
    return true;
  }
  if (!exact) {
    return hasInt16();
  }
}

bool Dimension::hasInt64  (bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::INT64) {
    return true;
  }
  if (!exact) {
    return hasInt32();
  }
}

bool Dimension::hasFloat32(bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::FLOAT32) {
    return true;
  }
  if (!exact) {
    return false;
  }
}

bool Dimension::hasFloat64(bool exact = false) {
  if (!hasValue()) {
    return false;
  }
  if (_parent->dimensions[_id]->type == DataPointType::FLOAT64) {
    return true;
  }
  if (!exact) {
    return hasFloat32();
  }
}

DataPointType Dimension::getType() {
  return _parent->dimensions[_id]->type;
}

void Dimension::clearValue() {
  _parent->dimensions[_id]->hasValue = false;
}

// Check if a value is present, and check/update timeout at the same time
bool Dimension::hasValue() {
  if (!(_parent->dimensions[_id]->hasValue)) {
    return false;
  }
  if (_parent->dimensions[_id]->hasTimeout && ((millis() - _parent->dimensions[_id]->lastTimestamp) > _parent->dimensions[_id]->timeoutInterval)) {
    _parent->dimensions[_id]->hasValue = false;
    return false;
  }
  return true;
}

int32_t Dimension::getTimeoutAge() {
  return _parent->dimensions[_id]->timeoutInterval;
}

int32_t Dimension::getCurrentAge() {
  return (millis() - _parent->dimensions[_id]->lastTimestamp);
}

void Dimension::setTimeoutAge(uint32_t timeoutAge = 0) {
  if (timeoutAge > 0) {
    _parent->dimensions[_id]->hasTimeout = true;
    _parent->dimensions[_id]->timeoutInterval = timeoutAge;
  } else {
    _parent->dimensions[_id]->hasTimeout = false;
    _parent->dimensions[_id]->timeoutInterval = 0;
  }
}

bool Dimension::hasNewValue() {
  return _parent->dimensions[_id]->hasNewValue;
}