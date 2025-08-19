drop table if exists
Workshop, Client, Mechanic, VehicleModel, Vehicle, SparePart, PartProvider, Ticket, SparePart_Buy,
Workshop_Client, Workshop_Mechanic, Client_Vehicle, Mechanic_Vehicle, VehicleModel_SparePart, SparePart_PartProvider;

create table Workshop(
    id int primary key auto_increment,
    name varchar(32),
    address varchar(98)
);

create table Client(
    id int primary key auto_increment,
    name varchar(32)
);

create table Mechanic(
    id int primary key auto_increment,
    name varchar (32)
);


create table VehicleModel(
    id int primary key auto_increment,    
    brand varchar(20),
    model varchar(32),
    year int
);

create table Vehicle(
    id int primary key auto_increment,    
    odometer int,
    vehiclemodel_id int,
    foreign key (vehiclemodel_id) references VehicleModel(id)
);

create table SparePart(
    id int primary key auto_increment,
    name varchar(32),
    used enum('No', 'Yes')
);

create table PartProvider(
    id int primary key auto_increment,
    name varchar(32)
);

create table Ticket(
    id int primary key auto_increment,
    price real,
    workshop_id int,
    client_id int,
    vehicle_id int,
    foreign key (workshop_id) references Workshop(id),
    foreign key (client_id) references Client(id),
    foreign key (vehicle_id) references Vehicle(id)
);

create table SparePart_Buy(
    amount int,
    sparepart_id int,
    partprovider_id int,
    ticket_id int,
    foreign key (sparepart_id) references SparePart(id),
    foreign key (partprovider_id) references PartProvider(id),
    foreign key (ticket_id) references Ticket(id)
);

create table Workshop_Client(
    workshop_id int,
    client_id int,
    foreign key (workshop_id) references Workshop(id),
    foreign key (client_id) references Client(id)
);

create table Workshop_Mechanic(
    workshop_id int,
    mechanic_id int,
    foreign key (workshop_id) references Workshop(id),
    foreign key (mechanic_id) references Mechanic(id)
);

create table Client_Vehicle(
    client_id int,
    vehicle_id int,
    foreign key (client_id) references Client(id),
    foreign key (vehicle_id) references Vehicle(id)
);

create table Mechanic_Vehicle(
    mechanic_id int,
    vehicle_id int,
    foreign key (mechanic_id) references Mechanic(id),
    foreign key (vehicle_id) references Vehicle(id)
);

create table VehicleModel_SparePart(
    vehiclemodel_id int,
    sparepart_id int,
    foreign key (vehiclemodel_id) references VehicleModel(id),
    foreign key (sparepart_id) references SparePart(id)
);

create table SparePart_PartProvider(
    sparepart_id int,
    partprovider_id int,
    price real,
    foreign key (sparepart_id) references SparePart(id),
    foreign key (partprovider_id) references PartProvider(id)
);