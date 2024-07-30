import csv

#requests.post(url="https://backend-nekansppvq-an.a.run.app/post_data/?bikeID=0")
bikeID=0
latitude=12.66552778
longitude=99.95838889
fallen=False

with open("./data/bicycleData.csv", "a", encoding="UTF-8") as file:
            data = {"bikeID": bikeID, "longitude": longitude, "latitude": latitude, "fallen": fallen}
            writer = csv.DictWriter(file, fieldnames=["bikeID", "longitude", "latitude", "fallen"])
            writer.writerow(data)

