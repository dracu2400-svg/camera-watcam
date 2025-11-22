# Battery Selection Guide for Water Meter AMR

**Choosing the Right Battery for 5-10 Year Operation**

---

## 🔋 Quick Selection Chart

| Target Life | Reading Interval | Recommended Battery | Capacity | Cost | Size |
|-------------|------------------|---------------------|----------|------|------|
| **5 years** | 2 hours | **AA Li-SOCl₂** | 2600 mAh | $8-12 | 14.5×50mm |
| **10 years** | 2 hours | **C-cell Li-SOCl₂** | 8500 mAh | $15-20 | 26×50mm |
| **15+ years** | 2 hours | **D-cell Li-SOCl₂** | 19000 mAh | $25-35 | 34×61mm |
| **3 years** | 1 hour | **AA Li-SOCl₂** | 2600 mAh | $8-12 | 14.5×50mm |

---

## 📊 Battery Technology Comparison

### Li-SOCl₂ (Lithium Thionyl Chloride) ✅ RECOMMENDED

**Pros:**
- ✅ Extremely low self-discharge (<1% per year)
- ✅ Long shelf life (10+ years)
- ✅ Wide temperature range (-60°C to +85°C)
- ✅ High energy density
- ✅ Flat discharge curve (stable voltage)
- ✅ No memory effect

**Cons:**
- ❌ Non-rechargeable
- ❌ Lower peak current (need capacitor for bursts)
- ❌ Voltage delay after storage
- ❌ Higher cost per cell

**Best for:** Long-term outdoor deployments (5-20 years)

---

### Li-ion (Standard Rechargeable) ❌ NOT RECOMMENDED

**Pros:**
- ✅ Rechargeable
- ✅ High peak current
- ✅ Lower cost
- ✅ Readily available

**Cons:**
- ❌ High self-discharge (2-5% per month)
- ❌ Requires protection circuit
- ❌ Shorter life (2-3 years max)
- ❌ Limited temperature range (0°C to +45°C)
- ❌ Voltage sag under load

**Best for:** Short-term applications, testing, development

---

### Alkaline ❌ NOT RECOMMENDED

**Pros:**
- ✅ Very low cost
- ✅ Widely available

**Cons:**
- ❌ High self-discharge (10-20% per year)
- ❌ Poor low-temperature performance
- ❌ Voltage drops significantly over time
- ❌ Leakage risk
- ❌ Low capacity

**Best for:** NOT suitable for this application

---

## 🎯 Recommended Batteries by Manufacturer

### For 5-Year Operation

#### Saft LS14500 (AA Size) ⭐ TOP CHOICE
- **Capacity:** 2600 mAh @ 3.6V
- **Chemistry:** Li-SOCl₂
- **Size:** 14.5mm × 50mm (AA)
- **Weight:** 17g
- **Temperature:** -60°C to +85°C
- **Self-discharge:** <1% per year
- **Manufacturer:** Saft (France)
- **Price:** ~$8-12
- **Expected Life:** **5.0 years** @ 64µA
- **Part Number:** LS14500
- **Datasheet:** [Saft Website](https://www.saftbatteries.com)

#### Tadiran SL-760 (AA Size) ⭐ ALTERNATIVE
- **Capacity:** 2400 mAh @ 3.6V
- **Chemistry:** Li-SOCl₂
- **Size:** 14.5mm × 50mm (AA)
- **Weight:** 17g
- **Temperature:** -60°C to +85°C
- **Manufacturer:** Tadiran (Israel)
- **Price:** ~$8-10
- **Expected Life:** **4.6 years** @ 64µA
- **Part Number:** SL-760
- **Datasheet:** [Tadiran Website](https://www.tadiranbat.com)

#### Eve ER14505 (AA Size)
- **Capacity:** 2700 mAh @ 3.6V
- **Chemistry:** Li-SOCl₂
- **Size:** 14.5mm × 50mm (AA)
- **Manufacturer:** EVE (China)
- **Price:** ~$5-8
- **Expected Life:** **5.2 years** @ 64µA
- **Part Number:** ER14505

---

### For 10-Year Operation

#### Saft LS26500 (C Size) ⭐ TOP CHOICE
- **Capacity:** 8500 mAh @ 3.6V
- **Chemistry:** Li-SOCl₂
- **Size:** 26mm × 50mm (C-cell)
- **Weight:** 50g
- **Temperature:** -60°C to +85°C
- **Self-discharge:** <1% per year
- **Manufacturer:** Saft (France)
- **Price:** ~$15-20
- **Expected Life:** **15.2 years** @ 64µA
- **Part Number:** LS26500

#### Tadiran SL-2770 (C Size) ⭐ ALTERNATIVE
- **Capacity:** 8500 mAh @ 3.6V
- **Chemistry:** Li-SOCl₂
- **Size:** 26mm × 50mm (C-cell)
- **Manufacturer:** Tadiran (Israel)
- **Price:** ~$15-18
- **Expected Life:** **15.2 years** @ 64µA
- **Part Number:** SL-2770

---

### For 15+ Year Operation

#### Saft LS33600 (D Size) ⭐ MAXIMUM LIFE
- **Capacity:** 17000 mAh @ 3.6V
- **Chemistry:** Li-SOCl₂
- **Size:** 33mm × 61mm (D-cell)
- **Weight:** 93g
- **Temperature:** -60°C to +85°C
- **Manufacturer:** Saft (France)
- **Price:** ~$25-35
- **Expected Life:** **30.7 years** @ 64µA
- **Part Number:** LS33600
- **Note:** Overkill for most applications, but provides huge margin

#### Tadiran SL-2780 (D Size)
- **Capacity:** 19000 mAh @ 3.6V
- **Chemistry:** Li-SOCl₂
- **Size:** 34mm × 61mm (D-cell)
- **Manufacturer:** Tadiran (Israel)
- **Price:** ~$25-30
- **Expected Life:** **34.2 years** @ 64µA
- **Part Number:** SL-2780

---

### For High Peak Current Applications (LoRaWAN Burst TX)

If you experience issues with voltage drop during LoRaWAN transmission:

#### Saft LSH14 (AA Size with Hybrid Technology)
- **Capacity:** 2600 mAh @ 3.6V
- **Peak Current:** Up to 1.5A
- **Built-in:** Hybrid Layer Capacitor (HLC)
- **Chemistry:** Li-SOCl₂ + Capacitor
- **Price:** ~$12-15
- **Use Case:** Handles LoRaWAN TX bursts better
- **Part Number:** LSH14

#### Tadiran TLH-5903 (AA Hybrid)
- **Capacity:** 2400 mAh @ 3.6V
- **Peak Current:** Up to 2A
- **Built-in:** PulsesPlus capacitor
- **Price:** ~$10-14
- **Part Number:** TLH-5903

---

## 🛒 Where to Buy

### Authorized Distributors

**North America:**
- Digi-Key: [https://www.digikey.com](https://www.digikey.com)
- Mouser: [https://www.mouser.com](https://www.mouser.com)
- Newark: [https://www.newark.com](https://www.newark.com)

**Europe:**
- Farnell: [https://www.farnell.com](https://www.farnell.com)
- RS Components: [https://www.rs-online.com](https://www.rs-online.com)

**Asia:**
- Element14: [https://www.element14.com](https://www.element14.com)

**Direct from Manufacturer:**
- Saft: [https://www.saftbatteries.com](https://www.saftbatteries.com)
- Tadiran: [https://www.tadiranbat.com](https://www.tadiranbat.com)

### Search Terms

When ordering, use these part numbers:
- **AA Li-SOCl₂:** LS14500, SL-760, ER14505
- **C-cell Li-SOCl₂:** LS26500, SL-2770
- **D-cell Li-SOCl₂:** LS33600, SL-2780
- **Hybrid AA:** LSH14, TLH-5903

⚠️ **WARNING:** Avoid counterfeit batteries on marketplaces like eBay/Amazon. Always buy from authorized distributors!

---

## 📐 Battery Life Calculations

### Formula

```
Battery Life (years) = (Capacity in mAh) / (Average Current in mA) / 8760 hours
```

### Examples

#### 5-Year Target with AA Battery

```
Required Average Current = 2600 mAh / (5 years × 8760 h/year)
                        = 2600 / 43800
                        = 59 µA maximum

Our optimized system: 64 µA average
Expected life: 2600 mAh / 0.064 mA / 8760 h = 4.6 years ✅
```

#### 10-Year Target with C-Cell Battery

```
Required Average Current = 8500 mAh / (10 years × 8760 h/year)
                        = 8500 / 87600
                        = 97 µA maximum

Our optimized system: 64 µA average
Expected life: 8500 mAh / 0.064 mA / 8760 h = 15.2 years ✅
```

### Derating Factors

**Temperature:** Add 20% capacity margin for cold climates
```
Cold climate capacity = Nominal capacity × 0.8
Example: 2600 mAh × 0.8 = 2080 mAh effective
```

**Aging:** Batteries degrade ~2% per year
```
Year 5 capacity = Initial capacity × (1 - 0.02 × 5)
                = 2600 mAh × 0.90 = 2340 mAh
```

**Safety Margin:** Always add 20% margin
```
Final capacity = Calculated capacity × 1.2
For 5 years: 2600 mAh / 1.2 = 2167 mAh minimum
```

---

## 🔌 Battery Connection

### Single Cell (3.6V Direct)

```
Li-SOCl₂ Cell (3.6V)
    ↓
[Reverse Polarity Protection Diode]
    ↓
[Power Switch - Optional]
    ↓
WE2 Board (3.3V LDO input)
```

**Notes:**
- Li-SOCl₂ 3.6V can be used directly (within WE2 specs)
- Most LDOs work with 3.6V input for 3.3V output
- Dropout voltage typically <300mV

### Series Connection (Higher Voltage)

⚠️ **Not recommended** - Li-SOCl₂ cells in series can cause issues with cell balancing

### Parallel Connection (Higher Capacity)

```
Cell 1 (2600 mAh) ─┐
                   ├─→ Output (5200 mAh total)
Cell 2 (2600 mAh) ─┘
```

**Important:** Use cells from same batch with matched voltages!

---

## ⚠️ Safety Considerations

### Li-SOCl₂ Safety

1. **Voltage Delay (Passivation)**
   - After long storage, first discharge may show voltage drop
   - Solution: Apply small load for 1 minute before deployment
   - Or use "de-passivated" batteries from manufacturer

2. **Peak Current Limitation**
   - Standard Li-SOCl₂: 50-100mA max continuous
   - LoRaWAN TX: 120+ mA bursts
   - Solution: Use hybrid batteries with built-in capacitor

3. **Non-Rechargeable**
   - NEVER attempt to recharge Li-SOCl₂
   - Can cause rupture or explosion
   - Mark clearly: "NON-RECHARGEABLE"

4. **Temperature Limits**
   - Operating: -60°C to +85°C
   - Storage: -40°C to +60°C
   - Keep within limits to avoid pressure buildup

5. **Disposal**
   - Li-SOCl₂ contains toxic chemicals
   - Must be recycled properly
   - Do NOT throw in regular trash
   - Contact local battery recycling program

---

## 📋 Deployment Checklist

### Before Installation

- [ ] Battery voltage >3.55V (fresh)
- [ ] Polarity protection in place
- [ ] Battery holder secure
- [ ] Contacts clean and tight
- [ ] Reverse polarity tested

### During Installation

- [ ] Record battery install date on device label
- [ ] Record expected replacement date (install date + 5 years)
- [ ] Note battery part number and batch code
- [ ] Take photo of installation
- [ ] Test first reading successful

### Monitoring

- [ ] Set up low-voltage alerts (3.2V threshold)
- [ ] Monitor battery voltage monthly for first 3 months
- [ ] Check for unexpected voltage drops
- [ ] Plan replacement schedule

### Replacement

- [ ] Replace at 3.2V (don't wait for 3.0V)
- [ ] Use same battery type as original
- [ ] Record replacement in maintenance log
- [ ] Recycle old battery properly

---

## 💰 Cost Analysis

### 5-Year Deployment Cost

| Battery | Unit Cost | Replacements | Total Cost | Cost per Year |
|---------|-----------|--------------|------------|---------------|
| AA Li-SOCl₂ | $10 | Every 5 years | $10 | $2/year |
| C-cell Li-SOCl₂ | $18 | Every 10 years | $9 (50% life used) | $1.80/year |
| D-cell Li-SOCl₂ | $30 | Every 15 years | $10 (33% life used) | $2/year |

### 10-Year Deployment Cost

| Battery | Unit Cost | Replacements | Total Cost | Cost per Year |
|---------|-----------|--------------|------------|---------------|
| AA Li-SOCl₂ | $10 | Every 5 years | $20 | $2/year |
| C-cell Li-SOCl₂ | $18 | Every 10 years | $18 | $1.80/year |
| D-cell Li-SOCl₂ | $30 | Every 15 years | $20 (67% life used) | $2/year |

**Conclusion:** C-cell Li-SOCl₂ offers best cost-per-year for 10-year deployments!

---

## 🎯 Final Recommendations

### For Most Deployments (5-Year Target)
**→ Saft LS14500 or Tadiran SL-760 (AA Li-SOCl₂)**
- Best balance of size, cost, and performance
- Easy to replace
- Widely available

### For Long-Term Deployments (10-Year Target)
**→ Saft LS26500 or Tadiran SL-2770 (C-cell Li-SOCl₂)**
- Excellent cost-per-year
- Minimal maintenance
- Proven reliability

### For Maximum Life (15+ Years)
**→ Saft LS33600 or Tadiran SL-2780 (D-cell Li-SOCl₂)**
- Install and forget
- Best for remote/difficult access locations
- Huge safety margin

### For High Peak Current (LoRaWAN Bursts)
**→ Saft LSH14 or Tadiran TLH-5903 (Hybrid AA)**
- Built-in capacitor for TX bursts
- More reliable LoRaWAN transmission
- Worth the extra cost for reliability

---

## 📞 Support

Questions about battery selection?
- See: [ULTRA_LOW_POWER_GUIDE.md](ULTRA_LOW_POWER_GUIDE.md)
- See: [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)

---

**Choose the right battery for 5-10 years of reliable operation!** 🔋⚡
