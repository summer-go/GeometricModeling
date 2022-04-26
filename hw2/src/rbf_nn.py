import tensorflow as tf
import numpy as np
# import matplotlib.pyplot as plt
import platform

class RBFLayer(tf.keras.layers.Layer):
    def __init__(self, num_outputs, kernel_initializer='glorot_uniform', bias_initializer='zeros'):
        print("---start RBFLayer")
        super(RBFLayer, self).__init__()
        self.num_outputs = num_outputs
        self.kernel_initializer = kernel_initializer
        self.bias_initializer = bias_initializer

    def build(self, input_shape):
        print("---build RBFLayer")

        self.kernel = self.add_weight(name='kernel', shape=(input_shape[-1], self.num_outputs),
                                      initializer=self.kernel_initializer)
        self.bias = self.add_weight(name='bias', shape=(self.num_outputs,), initializer=self.bias_initializer)

    def call(self, input):
        print("---call RBFLayer")
        x = tf.matmul(input, self.kernel) + self.bias
        return tf.exp(-tf.pow(x, 2) / 2)


def train(X, Y, num_middle, epochs, X_p):
    model = tf.keras.Sequential()
    model.add(tf.keras.layers.Input(shape=(1,), ))
    layer1 = RBFLayer(num_middle)
    model.add(layer1)
    layer2 = tf.keras.layers.Dense(1)
    model.add(layer2)

    print(model.summary())
    model.compile(loss='mean_squared_error')

    X_arr = np.array(X, dtype=np.float32)
    Y_arr = np.array(Y, dtype=np.float32)
    max_x = max(X_arr)
    max_y = max(Y_arr)
    X_arr /= max_x
    Y_arr /= max_y

    model.fit(X_arr, Y_arr, epochs=epochs, verbose=2)

    X_p_arr = np.array(X_p, dtype=np.float32)
    X_p_arr /= max_x
    Y_p_arr = model.predict(X_p_arr)
    Y_p_arr *= max_y
    Y_p = np.transpose(Y_p_arr)[0].tolist()

    return Y_p


if __name__ == '__main__':
    X = [-100, -50, 0.0, 50, 100]
    Y = [100, -100, -100, 100, 100]
    X_p = [i for i in range(-100, 101)]

    num_middle = 4
    epochs = 2000

    Y_p = train(X, Y, num_middle, epochs, X_p)

    # plt.plot(X, Y, color='red')
    # plt.plot(X_p, Y_p, color='blue')
    # plt.show()
