# CycleNet

*Description* 
<br>
An IoT innovation made with Arduino in order to facilitate the formation of a bicycle positioning and safety network for use in hotels in order to help make hotel bicycle renting to tourists more convienient and safe for both parties.
This project was made specifically for use at the 2024 Huahin Youth Innovation Competition. <br><br>


*Components(Prototype)* 
<br>
All of the components listed are all used with the Arduino microprocessor as this product prototype needs to utilize an Arduino for the microprocessor as per the requisites for the competition.
In truth, the Arduino and its modules is not the only component in this. As is listed down in the Program Structure, you will see that in actuality, the most important parts which allow this product to come into fruition is the code behind the scenes of it all. 
- Arduino Uno R4
    - NEO 6M GPS (module)
    - GY521 MPU6050 Accelerometer/Gyroscope (module)
    - SIM 800L GSM (module) - NOT CURRENTLY INSTALLRD IN PROTOTYPE
- Jumper Wires
- Breadboard
- Button(12x12x5mm)
- Bicycle Water Bottle(used for case)
- Screen
<br><br>

*Program Structure*
<br>
The program would best be described in 3 stages: the backend(api server), Arduino(on bicycle), and the website frontend(for hotels to manage).
- Backend: facilitates the transfer of information from the Arduino units that would be placed on the bicycles to the hotel's manager page. 
- Frontend: the main UI for the hotel staff. It's a simple web page that will display the information about all the bicycles in the network. The hotel is able to monitor whether or not the bicycle user requires assistance and where the current location of the bicycle is.
- Arduino: sends information regarding location and user status(whether they need assistance or not) while also monitoring for fall detection.
This will be the main UI for the User in which they will be able to relay if they require assistance to the hotel staff along with the safety features such as automatic fall detection.
<br><br>

*Miscellaneous notes* 
<br>
- The fall detection algorithm is inside the main C++ code for the Arduino(it's marked as a comment).
- The backend server is coded in Python with the FastAPI module and is hosted off of Google's Cloud Run service.
- The frontend website is coded in HTML along with some style and component libraries such as Bootstrap and (the thing used for the map, check in code).


