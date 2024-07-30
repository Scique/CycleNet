import csv

bikeID=0
longitude=12.322424
latitude=13.2489234989
fallen=False


with open("./data/bicycleData.csv", "r+", encoding="UTF-8") as file:
            reader = csv.DictReader(file, fieldnames=["bikeID", "longitude", "latitude", "fallen"])
            oldData = [row for row in reader]
            print(oldData)
            oldData.append({"bikeID": bikeID, "longitude": longitude, "latitude": latitude, "fallen": fallen})
            writer = csv.DictWriter(file, fieldnames=["bikeID", "longitude", "latitude", "fallen"])
            print(oldData)
            writer.writerows(oldData)