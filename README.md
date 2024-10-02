# Torudo

**Torudo** is a command-line tool designed to route your network traffic through the Tor network, allowing you to run any command or application anonymously. It also features packet capturing, identity management, and various other enhancements that can be enabled through a customizable build process.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Options](#options)
- [Enabling Features](#enabling-features)
- [Packet Capturing](#packet-capturing)
- [Dependencies](#dependencies)
- [Contributing](#contributing)

---

## Features

- **Route Commands Through Tor**: Send any command or application’s traffic through the Tor network to ensure anonymity.
- **Identity Management**: Save, load, and list Tor identities for different sessions.
- **Geolocation Filtering**: Set preferred exit nodes by specifying countries (e.g., `us`, `de`).
- **Command Sequencing**: Run a series of commands with each command having its own Tor identity.
- **Circuit Quality Assessment**: Assess the quality of the Tor circuit and switch exit nodes if necessary.
- **Custom Routing Rules**: Set custom IP routing rules for certain destinations to bypass Tor.
- **Packet Capturing**: Capture network traffic of any command sent through the Tor network, saving the capture in a `.pcap` file for later analysis.

---

## Installation

### Clone the Repository
```bash
git clone https://github.com/nethq/torudo.git
cd torudo
```

### Install Required Dependencies

1. **json-c**: Required for identity management functionality.
    ```bash
    sudo apt install libjson-c-dev
    ```

2. **tcpdump**: Required for packet capturing functionality.
    ```bash
    sudo apt install tcpdump
    ```

3. **Tor**: Required to route traffic through the Tor network.
    ```bash
    sudo apt install tor
    ```

### Compile the Project

The `Makefile` allows you to choose the features you want to enable or disable at compile time.

```bash
make
```

During the build process, you will be prompted to enable or disable features such as verbose logging, identity management, packet capturing, and others. After selecting your options, the binary will be compiled.

---

## Usage

Once compiled, you can use **Torudo** by running the `torudo` binary followed by the command you want to execute through the Tor network. For example:

```bash
sudo ./torudo curl ifconfig.io
```

This will route the `curl` request through the Tor network and return an IP address different from your actual IP.

### Example Command

```bash
sudo ./torudo curl https://example.com
```

This sends the `curl` command through the Tor network, allowing you to retrieve a webpage anonymously.

---

## Options

- `--help`: Display the help message.

If you enabled **identity management**, you will also have these options:

- `--save-identity <name>`: Save the current Tor identity.
- `--load-identity <name>`: Load a previously saved Tor identity.
- `--list-identities`: List all saved identities.

If you enabled **command sequencing**, you will have the option:

- `--sequence <command1> <command2> ...`: Execute a sequence of commands, with each command using a different Tor identity.

---

## Enabling Features

During the build process, you will be asked if you want to enable certain features. These are the available features:

- **Verbose Logging**: Provides detailed logs about what Torudo is doing.
- **Identity Manager**: Allows saving and loading different Tor identities.
- **Command Sequence**: Run a series of commands with different Tor identities.
- **Geolocation Filter**: Set specific countries for Tor exit nodes.
- **Tor Monitor**: Monitor the activity of Tor, including uptime and the current exit node IP.
- **Circuit Assessment**: Assess and switch Tor circuits if the quality is too low.
- **Custom Routing**: Set custom routing rules to bypass Tor for certain IP addresses.
- **Packet Capturing**: Capture all traffic sent through the Tor network and save it as a `.pcap` file.

---

## Packet Capturing

If you enabled packet capturing, Torudo will automatically start capturing traffic for the command you execute and store it in a `.pcap` file located at:

```
/tmp/torudo_capture.pcap
```

You can analyze this file using **Wireshark**:

```bash
wireshark /tmp/torudo_capture.pcap
```

To disable packet capturing, simply recompile the project without enabling the packet capture feature.

---

## Dependencies

The following dependencies are required based on the features you enable:

- **Tor**: The core routing network.
  - Install with:
     ```bash
     sudo apt install tor
     ```

- **libjson-c**: Required for identity management.
  - Install with:
     ```bash
     sudo apt install libjson-c-dev
     ```

- **tcpdump**: Required for packet capturing.
  - Install with:
     ```bash
     sudo apt install tcpdump
     ```

---

## Contributing

Contributions are welcome! If you would like to contribute, please fork the repository and create a pull request with your improvements.

---

If this effort proved useful to you, internet starry imaginary points are always welcome!