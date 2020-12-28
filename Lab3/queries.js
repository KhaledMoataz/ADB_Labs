//1
mongoimport --db lab3 --collection restaurants restaurants.json

//2
db.restaurants.find().pretty()

//3
db.restaurants.find({},{_id:0,restaurant_id:1,name:1,borough:1,"address.zipcode":1}).pretty()

//4
db.restaurants.find({"borough":"Bronx"}).limit(5).pretty()

//5
db.restaurants.find({"borough":"Bronx"}).skip(5).limit(5).pretty()

//6
db.restaurants.find( {
                    $and : [
                        {"cuisine":{$ne:"American "}}, 
                        {"grades.score":{$gt:70}}, 
                        {"address.coord.0":{$lt:-65.754168}}
                    ]
                    }).pretty()

//7
db.restaurants.find( {
                    $and : [
                        {"borough":"Bronx"}, 
			{ $or : [
				{"cuisine" : "American "},
				{"cuisine" : "Chinese"}
				]
			}
                    ]
                    }).pretty()

//8
db.restaurants.find({"grades.score":{$not :{$gt:10}}},{_id:0,restaurant_id:1,name:1,borough:1,cuisine:1}).pretty()

//9
db.restaurants.find({"grades.score":{$mod: [7, 0]}},{_id:0,restaurant_id:1,name:1,grades:1}).pretty()

//10
db.restaurants.find(
                { $and : [
                      {"address.coord.1" : {$gt:42}} ,
                      {"address.coord.1" : {$lte:52}} 
                      ]
                },
                {"restaurant_id":1,"name":1,"address":1}
                ).pretty()

