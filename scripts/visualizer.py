#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Imu
import matplotlib.pyplot as plt
from collections import deque

class SimpleZPlot(Node):
    def __init__(self):
        super().__init__('simple_z_plot')
        self.t = deque(maxlen=200)
        self.z = deque(maxlen=200)
        self.start = self.get_clock().now().nanoseconds / 1e9
        
        self.sub = self.create_subscription(Imu, '/imu', self.cb, 10)
        
        # Simple static plot that updates
        plt.ion()  # Interactive mode
        self.fig, self.ax = plt.subplots(figsize=(10, 5))
        self.ax.set_title('IMU Z Component Over Time')
        self.ax.set_xlabel('Time (s)')
        self.ax.set_ylabel('Z Value')
        self.ax.grid(True)
        
        print("Plotting Z component. Close window to stop.")
    
    def cb(self, msg):
        self.t.append(self.get_clock().now().nanoseconds / 1e9 - self.start)
        self.z.append(msg.orientation.z)
        
        # Update plot
        self.ax.clear()
        self.ax.plot(list(self.t), list(self.z), 'b-', linewidth=2)
        self.ax.set_title(f'IMU Z Component (Current: {msg.orientation.z:.6f})')
        self.ax.set_xlabel('Time (s)')
        self.ax.set_ylabel('Z Value')
        self.ax.grid(True)
        
        # Show last 30 seconds
        if len(self.t) > 1:
            self.ax.set_xlim(max(0, self.t[-1] - 30), self.t[-1])
        
        # Auto-scale y
        if self.z:
            margin = max(0.02, abs(max(self.z) - min(self.z)) * 0.3)
            self.ax.set_ylim(min(self.z) - margin, max(self.z) + margin)
        
        plt.pause(0.01)  # Small pause to update plot

def main():
    rclpy.init()
    node = SimpleZPlot()
    rclpy.spin(node)
    plt.close('all')

if __name__ == '__main__':
    main()
