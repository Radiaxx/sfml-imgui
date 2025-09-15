import numpy as np
import matplotlib.pyplot as plt

def readAscFile(filepath):
    """
    Reads an .asc grid file.
    Returns:
        A tuple containing:
        - data (np.ndarray): A 2D NumPy array of the scalar data.
        - header (dict): A dictionary containing the header metadata.
    """
    header = {}
    try:
        with open(filepath, 'r') as f:
            # Read header
            for _ in range(6):
                line = f.readline().strip()
                parts = line.split()
                key = parts[0].lower()
                value = float(parts[1])
                header[key] = value

            # Read data
            data = np.loadtxt(f)

    except FileNotFoundError:
        print(f"File not found at '{filepath}'")
        return None, None
    except Exception as e:
        print(f"An error occurred while reading the file: {e}")
        return None, None

    # Validate dimensions
    nrows, ncols = int(header.get('nrows')), int(header.get('ncols'))
    if data.shape != (nrows, ncols):
        print(f"Data dimensions {data.shape} do not match header ({nrows}, {ncols}).")
        return None, None
        
    return data, header

def plotAscHeatmap(filepath, colormapName):
    """
    Reads an .asc file and plots it as a heatmap using a specific colormap.
    """
    data, header = readAscFile(filepath)

    if data is None or header is None:
        return

    nodataValue = header.get('nodata_value', -9999.0)

    # Create a masked array to properly handle NODATA values.
    # This prevents the NODATA value from skewing the color scale.
    maskedData = np.ma.masked_where(data == nodataValue, data)

    fig, ax = plt.subplots(figsize=(10, 8))
    im = ax.imshow(maskedData, cmap=colormapName, origin='upper')

    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label('Scalar Value')

    ax.set_title(f"Heatmap of '{filepath}'\nColormap: '{colormapName}'")
    ax.set_xlabel("Columns")
    ax.set_ylabel("Rows")
    
    plt.show()

if __name__ == "__main__":
    ascFilePath = '../data/aletsch_32T.asc'
    colormap = 'gist_earth' # You can change this to any valid Matplotlib colormap name

    plotAscHeatmap(ascFilePath, colormap)
