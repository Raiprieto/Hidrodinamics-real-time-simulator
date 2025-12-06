import pandas as pd
import matplotlib.pyplot as plt

# 1. Cargar convergencia y energía
df = pd.read_csv("build/simulation_log.csv") # Ajusta la ruta

plt.figure(figsize=(10, 5))

# Graficar Energía Cinética Total
plt.subplot(1, 2, 1)
plt.plot(df['Step'], df['KineticEnergy'])
plt.title("Energía Cinética Total (Estabilidad)")
plt.xlabel("Pasos de Tiempo")
plt.ylabel("Energía")

# Graficar Masa Total (Debe ser constante)
plt.subplot(1, 2, 2)
plt.plot(df['Step'], df['TotalMass'])
plt.title("Conservación de Masa")
plt.ylim(df['TotalMass'].min() * 0.99, df['TotalMass'].max() * 1.01) # Zoom para ver errores pequeños

plt.tight_layout()
plt.show()