import pandas
import numpy
import ROOT
import uproot
import argparse
import numpy as np
import pandas as pd

parser = argparse.ArgumentParser(description="Obtain efficiency to photons from Geant4.")
parser.add_argument("gas", help="Gases Concentrations He CF4 Ar",nargs=3, type=int)
parser.add_argument("--infile", help="root file from Geant4",default="../build/output.root")
args = parser.parse_args()

def create_and_fill_histogram(df, column_name, bin_edges,name):
    # Create a TH1D histogram
    n_bins = len(bin_edges) - 1
    hist = ROOT.TH1D(name, name, n_bins, bin_edges)
    # Fill the histogram with values from the specified column
    for value in df[column_name]:
        hist.Fill(value)
    #hist.Write()
    return hist
def create_ratio_graph(genHist, intHist):
    # Number of bins
    n_bins = genHist.GetNbinsX()
    # Arrays for TGraphErrors
    x = np.zeros(n_bins)
    y = np.zeros(n_bins)
    ex = np.zeros(n_bins)
    ey = np.zeros(n_bins)
    for i in range(1, n_bins + 1):
        # Bin centers
        x[i-1] = genHist.GetBinCenter(i)
        # Bin widths (assuming uniform bins)
        ex[i-1] = genHist.GetBinWidth(i) / 2
        # Bin contents
        gen_entries = genHist.GetBinContent(i)
        int_entries = intHist.GetBinContent(i)
        if gen_entries > 0:
            # Ratio
            y[i-1] = int_entries / gen_entries
            # Errors on bin contents (assuming Poisson statistics)
            gen_error = np.sqrt(gen_entries)
            int_error = np.sqrt(int_entries)
            # Error propagation
            if int_entries > 0:
                ey[i-1] = y[i-1] * np.sqrt((gen_error/gen_entries)**2 + (int_error/int_entries)**2)
            else:
                ey[i-1] = 0
        else:
            y[i-1] = 0
            ey[i-1] = 0
    # Create TGraphErrors
    #graph = ROOT.TGraphErrors(n_bins, x*1000, y, ex, ey)
    #graph.Write()
    return x*1000, y, ex*1000, ey
def style_and_draw_graph(graph, gas="HeCF_{4} 60/40",color=ROOT.kRed - 4):
    # Apply styles to the graph
    graph.SetNameTitle("Efficiency plot", "")
    graph.GetXaxis().SetTitle("Energy (keV)")
    graph.GetYaxis().SetTitle("Photoelectric Interaction Probability")
    graph.SetMarkerColor(color)  # blue
    graph.SetLineColor(color)
    graph.SetMarkerStyle(8)
    graph.SetMarkerSize(2)
    graph.Write()
    # Create a canvas
    canvas = ROOT.TCanvas("EfficiencyPhotoelectric", "EfficiencyPhotoelectric", 1000, 1000)
    canvas.SetLeftMargin(0.15)
    canvas.SetRightMargin(0.05)
    canvas.SetTopMargin(0.05)
    # Set log scale on the y-axis
    canvas.SetLogy()
    # Draw the graph on the canvas
    graph.Draw("AP")  # "A" for axis, "P" for points
    # Set the y-axis range
    if args.gas[0]==0: graph.GetYaxis().SetRangeUser(1e-3, 1e-0)
    else: graph.GetYaxis().SetRangeUser(1e-4, 1e-1)
    # Create and draw TPaveText
    pave_text = ROOT.TPaveText(0.65, 0.8, 0.8, 0.95, "NDC")
    pave_text.AddText("Gas cube with 10cm side")
    pave_text.AddText(f"{gas} at 1atm")
    pave_text.SetFillColor(0)  # Transparent fill
    pave_text.SetFillStyle(0)  # Transparent fill style
    pave_text.SetBorderSize(0)  # No border
    pave_text.SetTextColor(ROOT.kBlack)  # Text color
    pave_text.SetTextSize(0.04)  # Text size
    pave_text.SetTextFont(42)  # Text font
    pave_text.Draw()
    # Update the canvas to display the graph
    canvas.Update()
    # Save the canvas if needed
    string=gas.replace("/", "_").replace(" ", "").replace("_", "").translate({ord("{"): None, ord("}"): None})
    canvas.SaveAs(f"EfficiencyPhotoelectric_{string}.png")
    canvas.Write()
    return canvas
# Open the ROOT file and access the TTree
file = uproot.open(args.infile)
tree = file['tree']

if args.gas[0]!=0:
    outfile=f"efficiency_He{args.gas[0]}{args.gas[1]}.root"
    name=f"HeCF_{{4}} {args.gas[0]}/{args.gas[1]}"
else:
    outfile=f"efficiency_Ar{args.gas[2]}{args.gas[1]}.root"
    name=f"ArCF_{{4}} {args.gas[2]}/{args.gas[1]}"

print("Opening TTree...")
# Convert the TTree into a pandas DataFrame
df = tree.arrays(library="pd")
#print(df)

energies=np.linspace(0.01,0.07,100)

#primary photon should have interacted
#primary photon should have interacted only once to be photoelectric
#Primary energy and deposited energy should be the same more or less
epsilon=0.0002#200eV
condition=(df["BoolInteracted"]==1) & (df["primaryHits"]==1 ) & (df["energyDeposit"]>df["primaryEnergy"]-epsilon)& (df["energyDeposit"]<=df["primaryEnergy"])

print("Cutting TTree...")
filtered_df = df[condition]

genHist=create_and_fill_histogram(df,"primaryEnergy",energies,"Generated Energies")
intHist=create_and_fill_histogram(filtered_df,"primaryEnergy",energies,"Interacted Energies")
# Get the data from the create_ratio_graph function
energy, efficiency, err_energy, err_efficiency = create_ratio_graph(genHist, intHist)
# Create a DataFrame
data = {
    'energy': energy,
    'efficiency': efficiency,
    'err_energy': err_energy,
    'err_efficiency': err_efficiency
}
df_eff = pd.DataFrame(data)

print("Writing TTrees...")
# Write the filtered DataFrame to a new ROOT file
with uproot.recreate(outfile) as new_file:
    new_file["cuttree"] = filtered_df
    new_file["OGtree"] = df
    new_file["efficiency"] = df_eff

main = ROOT.TFile(outfile, "UPDATE")

genHist.Write()
intHist.Write()
style_and_draw_graph(ROOT.TGraphErrors(len(energy), energy, efficiency, err_energy, err_efficiency),gas=name)