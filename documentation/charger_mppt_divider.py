V_oc = 22.6
V_mp = 17.5 # central 25°C til 50°C MPP
V_bat_float = 4
V_bat_min = 2.5

K_r = 0.1 / (V_mp / V_oc) # R1 ja R2 
print("Suhe", K_r)

R_mppt1 = 100000

R_mppt2 = (0.1 /((V_mp/V_oc)-0.1) * R_mppt1)

print(R_mppt2)

R_mppt2 = 100000
R_mppt1 = ((1-K_r) / K_r) * R_mppt2
#R_mppt1 = ((1-K_r) / K_r) * 121000
print("R_mppt1", R_mppt1)
print("R_mppt2", R_mppt2)

print("Feedback")

V_FB = 2.4

print("V_float =", V_FB * (1010+1420)/1420)
print("V_float =", V_FB * (4700+9220)/9220)

CL = 12.5
Cs = 5
Ca = Cb = 15
print(((Ca * Cb) / (Ca + Cb)) + Cs)
