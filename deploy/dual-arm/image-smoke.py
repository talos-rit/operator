"""Offline image/model test. No camera, socket, or robot is opened."""
import numpy as np
import cv2
import onnxruntime
from pose import PoseDetector
from inference_rate import InferenceRate

model = PoseDetector('/opt/talos/model.onnx', conf_thresh=0.35)
result = model.detect(np.zeros((320, 320, 3), dtype=np.uint8))
rate = InferenceRate(2)
rate.started()
assert 0 < rate.delay() <= 0.5
print({'opencv': cv2.__version__, 'onnxruntime': onnxruntime.__version__,
       'detections': len(result), 'inference_fps': 2, 'robot_devices': 'none'})
