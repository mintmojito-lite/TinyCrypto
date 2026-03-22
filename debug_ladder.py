import sys

def hex_to_int(h): 
    # Decode RFC 7748 hex strings which are Little-Endian representation (byte 0 is left-most)
    b = bytes.fromhex(h)
    return int.from_bytes(b, 'little')

def int_to_hex(i): 
    return i.to_bytes(32, 'little').hex()

# Alice
scalar = hex_to_int("77076d0a7318a57d3c16c17251b26645cefaeba7c5d1f9da0ee37e71ad5f5fae")
u = 9
P = 2**255 - 19
A24 = 121665

# Clamp
scalar &= ~(7)
scalar &= ~(128 << (31 * 8))
scalar |= (64 << (31 * 8))

x_1 = u
x_2 = 1
z_2 = 0
x_3 = u
z_3 = 1

swap = 0
print(f"Initial: x2={x_2}, z2={z_2}, x3={x_3}, z3={z_3}")

for t in range(254, -1, -1):
    k_t = (scalar >> t) & 1
    swap ^= k_t
    if swap:
        x_2, x_3 = x_3, x_2
        z_2, z_3 = z_3, z_2
    swap = k_t

    A = (x_2 + z_2) % P
    AA = (A * A) % P
    B = (x_2 - z_2) % P
    BB = (B * B) % P
    E = (AA - BB) % P
    C = (x_3 + z_3) % P
    D = (x_3 - z_3) % P
    DA = (D * A) % P
    CB = (C * B) % P
    x_3 = ((DA + CB) * (DA + CB)) % P
    z_3 = (x_1 * ((DA - CB) * (DA - CB))) % P
    x_2 = (AA * BB) % P
    z_2 = (E * (AA + A24 * E)) % P
    
    if t >= 253:
        print(f"t={t}: swap={k_t}")
        print(f"x2={hex(x_2)}, z2={hex(z_2)}")
        print(f"x3={hex(x_3)}, z3={hex(z_3)}")

inv = pow(z_2, P - 2, P)
out = (x_2 * inv) % P
print(f"Out: {int_to_hex(out)}")
