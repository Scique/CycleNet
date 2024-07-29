from fastapi import FastAPI
import csv
from mangum import Mangum

# This code will be upload to AWS Lambda for execution and as such will not run locally on the device
# Using fastapi for the backend web api

# Create the api instance
app = FastAPI()

# Create a handler for the aws lambda function
handler = Mangum(app)


# For testing and to tell people where to find the documentation
@app.get("/")
async def noreq():
    return {"message": "Visit the /docs page for documentation. "}


# Post location and bicycle status data from Arduino
@app.post("/post_data/")
async def postData(bikeID: int, longitude:float, latitude: float, fallen: bool):
    # Write the data to the csv file
    try:
        with open("./data/bicycleData.csv", "a", encoding="UTF-8") as file:
            data = {"bikeID": bikeID, "longitude": longitude, "latitude": latitude, "fallebn": fallen}
            writer = csv.DictWriter(file, fieldnames=["bikeID", "longitude", "latitutde", "fallen"])
            writer.writerow(data)
        return {"status": 201, "message": "Data upload successful. "}
    except Exception as e:
        return {"status": 422, "message": e}
    

# Get location and bicycle data from the server
@app.get("/get_data/")
# Get the latest data for the particular bike
# If ID not provided, will send all the data
async def getData(bikeID: int | None = None):
    with open("./data/bicyleData.csv", "r", encoding="UTF-8") as file:
        reader = csv.DictReader(file, fieldnames=["bikeID", "longitude", "latitutde", "fallen"])
        try:
            if bikeID != None:
                # Make sure the bikeID provided is inside the database
                useData = [row for row in reader if row["bikeID"] == bikeID]
            else:
                # All data
                useData = [row for row in reader]
            return {"status": 200, "message": "Data successfully retrieved and sent to user", "content": useData}
        except:
            return {"status": 404, "message": "The requested bike was not found. "}
            
    
    



    

    


