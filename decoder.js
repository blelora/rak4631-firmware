function Decoder(payload, port) {

    var decoded = [];

    switch (port) {
        case 1: // Geo Location
            var lat = (payload[3] << 24 | payload[2] << 16 | payload[1] << 8 | payload[0]) / 10000000;
            var long = (payload[7] << 24 | payload[6] << 16 | payload[5] << 8 | payload[4]) / 10000000;
            var accuracy = payload[8];
            decoded.push({
                field: 'LATITUDE',
                value: lat
            });
            decoded.push({
                field: 'LONGITUDE',
                value: long
            });
            var location = "("+lat+","+long+")";
            decoded.push({
                field: 'LOCATION',
                value: location
            });
            decoded.push({
                field: 'ACCURACY',
                value: accuracy
            });
            break;
        case 2: // Battery Data
            var capacity = payload[0];
            var charging_state = payload[1];
            decoded.push({
                field: 'BATTERY_CAPACITY',
                value: capacity
            });
            decoded.push({
                field: 'CHARGING_STATE',
                value: charging_state
            });
            break;
        case 3: // Vechicle Data
            var vehicle_speed = payload[0];
            var vehicle_rpm = payload[2] << 8 | payload[1];
            var vin = String.fromCharCode.apply(null, payload.slice(3, 19));
            decoded.push({
                field: 'VEHICLE_SPEED',
                value: vehicle_speed
            });
            decoded.push({
                field: 'VEHICLE_RPM',
                value: vehicle_rpm
            });
            decoded.push({
                field: 'VIN',
                value: vin
            });
            break;
    }

    return decoded;

    /*
    return [
        {
            field: "TEST",
            value: 123
        }
    ];
    */
}