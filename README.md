# Fractional-calculations
Implementation of fixed and floating point based on integer calculations

Данная программа производит вычисление суммы/разности/произведения/частного двух дробных чисел, заданных в форматах фиксированной точки и плавающей точки.
При этом в вычислениях **используются только целочисленные операции и типы данных**.

Фиксированная точка задается форматом (число_бит_целой_части.число_бит_дробной_части), и числовым значением в шестнадцатеричной системе счисления.
Общий размер числа не более 32 бит. При этом при умножении и делении сохраняется знак, то есть произведение положительного и отрицательного числа **всегда** отрицательно.

Плавающая точка реализована согласно стандарту IEEE-754.
Поддерживаются два формата: half precision и single precision.

Округления производятся к нулю.
