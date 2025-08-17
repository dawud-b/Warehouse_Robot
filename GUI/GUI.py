# GUI.py
#
# Created: November 21st, 2024
# Author: Connor Grim
# GUI For Final Project

import tkinter as tk
import customtkinter as ctk
import socket
import threading
import random

depot_num = -1
cybot_started = False
depot_button_pressed = False

#Open socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(('192.168.1.1', 288))
depotButtons = []

#Updates Status Text Box
def update_status(msg):
    global status_textbox
    status_textbox.delete(1.0, tk.END)
    status_textbox.insert(tk.END, msg)
    status_textbox.see(tk.END)

#Receive Data from CyBot
def receive_data(client_socket, buttons):
    while True:
        try:
            data = client_socket.recv(1024)
            if not data:
                break
            received_data = data.decode().strip()
            print("Received from server: " + received_data)

            # Received a byte = Cybot has reached delivery zone from start position, unlocked depot buttons for selection
            if received_data == "a":
                update_status("Cybot Has Reached Delivery Zone, Please Select A Depot...")
                def unlock_buttons():
                    for button in buttons:
                        button.configure(state=tk.NORMAL)
                        print(f"Button {button.cget('text')} unlocked") # Debugging: Confirm button is unlocked
                app.after(0, unlock_buttons)

                buttons[0].configure(command=lambda: depot_button_clicked("1"))
                buttons[1].configure(command=lambda: depot_button_clicked("2"))
                buttons[2].configure(command=lambda: depot_button_clicked("3"))
                buttons[3].configure(command=lambda: depot_button_clicked("4"))

            # Received f byte = Cybot has reached delivery zone with depot item
            elif received_data == "f":
                update_status("Cybot Has Delivered, Select Another Depot...")

            elif received_data == "n":
                global depot_num
                update_status("Depot " + depot_num + " is unavailable at this time. Please try again later.")

            # Received byte to update bot coords in map
            elif received_data[0:2] == "b(":
                received_data = received_data[1:].strip("()")
                print(received_data)
                x_str, y_str = received_data.split(",")
                x = int(x_str) / 10
                y = (2432 - int(y_str)) / 10

                print("x: " + str(x) + " y: " + str(y))
                update_cybot(x, y)

            # Received byte to add obstacle edge dot to map
            elif received_data[0:2] == "e(":
                coordinates = received_data.split("\n")

                for coordinate in coordinates:
                    coordinate = coordinate[1:].strip("()")
                    x_str, y_str = coordinate.split(",")
                    x = int(x_str) / 10
                    y = (2432 - int(y_str)) / 10
                    print("x: " + str(x) + " y: " + str(y))
                    canvas.create_oval(x, y, x + 5, y + 5, fill="red", tags="obstacle")
                    update_status("Cybot Detected Obstacle!")

        except Exception as e:
            print(e)
            break

# Create the main application window
app = ctk.CTk()
app.title("Cybot GUI")
app.geometry("800x600")

# Create a canvas
canvas = ctk.CTkCanvas(app, width=450, height=250, bg="black")
canvas.pack()

cybotX = 18
cybotY = 243.2 - 18

canvas.create_oval(212.8 - 15, 243.2 - 213.2 - 15, 212.8 + 15, 243.2 - 213.2 + 15, outline="blue", tags="depot")
canvas.create_oval(cybotX - 17.5, cybotY - 17.5, cybotX + 17.5, cybotY + 17.5, outline="white", tag="cybot")

# Update coordinate of cybot
def update_cybot(newX, newY):
    canvas.delete("cybot")
    global cybotX, cybotY
    cybotX = newX
    cybotY = newY
    canvas.create_oval(cybotX - 17.5, cybotY - 17.5, cybotX + 17.5, cybotY + 17.5, outline="white", tag="cybot")

# Send start byte to CyBot
def start_cybot():
    global cybot_started, startCybotButton, cybot_started
    data = 's'
    client_socket.send(data.encode())
    if (cybot_started == False):
        update_status("Starting Cybot...")
        startCybotButton.configure(text="Stop Cybot")
        cybot_started = True
        print("Sent to server: " + data + "\n")
    else:
        update_status("Stopping Cybot...")
        startCybotButton.configure(text="Start Cybot")
        cybot_started = False
        print("Sent to server: " + data + "\n")

# Send depot byte when a depot button is clicked
def depot_button_clicked(num):
    global depot_num, depot_button_pressed
    depot_num = num
    client_socket.send(num.encode())

    if (depot_button_pressed == False):
        update_status("Going to Depot " + str(num) + "...")
        depot_button_pressed = True
    else:
        update_status("Returning to Delivery Zone...")
        depot_button_pressed = False
    print("Sent to server: " + num + "\n")
# Add a button to place random dots
startCybotButton = ctk.CTkButton(app, text="Start Cybot", command=start_cybot)
startCybotButton.pack(pady=20, padx=100)

button_frame = ctk.CTkFrame(app)

for i in range(4):
    button = ctk.CTkButton(button_frame, text=str(i + 1), state=ctk.DISABLED)
    button.grid(row=0, column=i, padx=5, pady=5)
    depotButtons.append(button)
button_frame.pack(padx=10)

status_textbox = tk.Text(app, height=5, font=("Courier New", 12), state=tk.NORMAL)
status_textbox.pack(pady=10)

receive_thread = threading.Thread(target=receive_data, args=(client_socket, depotButtons))
receive_thread.start()
# Run the application
app.mainloop()

client_socket.close()