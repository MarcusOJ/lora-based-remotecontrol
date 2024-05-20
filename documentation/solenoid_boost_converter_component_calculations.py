V_in_min = 3
V_s_out = 12
I_lim_min = 3.2
fs = 1.2 * 10 ** 6
L = 3.3 * 10 ** -6
n = 0.9
#I_out_max = 0.3

"""
The first step in the design procedure is to verify that the maximum possible output current of the boost converter
supports the specific application requirements.
"""
print("### Design requirements ###")
# 1. Duty cycle
D = 1 - (V_in_min * n)/(V_s_out)
print("Duty cycle", D, "%")

# Inductur peak-to-peak ripple current
delta_I_l = (V_in_min * D) / (fs * L)
print("Inductur peak-to-peak ripple current", round(delta_I_l, 2), "A")

# 2. Maximum output current, I_out_max
I_out_max = (I_lim_min - (delta_I_l)/(2)) * (1 - D)
print("Maximum output current", round(I_out_max,2), "A")

# 3. Peak switch current in application, I_sw_peak
I_sw_peak = (delta_I_l)/(2) + (I_out_max)/(1 - D)
print("Peak switch current in application", round(I_sw_peak, 2), "A")


"""
Inductor Selection
TI recommends an inductor current ripple below 35% of the average inductor current. Therefore, the
following equation can be used to calculate the inductor value.
"""
print("\n### Inductor selection ###")
I_out_max = 0.46

L = (( (V_in_min) / (V_s_out) )**2 ) * ( (V_s_out - V_in_min) / (I_out_max * fs)) * (n / 0.35)
print("Inductor value", L * 10 ** 6, "uH")


"""
Setting the output voltage
"""
print("\n### Output voltage ###")

V_fb = 1.238
R2 = 18000 # as per documentation

R1 = R2 * ((V_s_out) / (V_fb) - 1)
print("R1", round(R1, 2), "ohm")

"""
Compensaiton Network Values
"""

# per table 5. in datasheet 
# 68kOHM 1.2nF



