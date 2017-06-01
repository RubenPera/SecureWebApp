/**
 * Created by ruben on 5/31/17.
 */

Vue.component('modal', {
    template: '#flightOverView-modal-template'
})

Vue.component('modal', {
    template: '#userinfo-modal-template'
})

$(document).ready(new Vue({
    el: '#masterPage',
    data: {
        Links: [],
        message: 'Hello Vue!',
        login: {
            email: "user2@teamalfa.com",
            password: "",
            error: "",
            emailError: "",
        },
        FlightOverView: {
            FlightItem: {
                external_id: 0,
                date: '',
                price: '',
                flight_source: '',
                flight_destination: '',
                capacity: 0,

            },
            Flights: [],
            showModal: false,
            query: '',
            query2: '',
        },
        UserInfo: {
            UserInfo: {
                email: '',
                inholland_miles: 0,
            },
            showModal: false,
            Flights: [],
            query: '',
            query2: '',

            ChangePass:
            {
                old: '',
                new: '',
                error: "",
            }


        },
        Admin: {
            Users: [],
        }


    },
    computed: {
        tableFilter: function () {
            return this.findBy(this.FlightOverView.Flights, this.FlightOverView.query, this.FlightOverView.query2, 'flight_source', 'flight_destination')
        },
        tableFilter2: function () {
            return this.findBy(this.UserInfo.Flights, this.UserInfo.query, this.UserInfo.query2, 'flight_source', 'flight_destination')
        }

    },
    mounted: function () {
        this.loadLinks();
        this.validateEmail();
        this.loadFlights();
        this.loadUser();
        this.adminLoadUsers();
    },
    methods: {
        loadLinks: function () {
            var _this = this;

            this.$http.get('/getLinks').then(response => {

                // get body data
                this.Links = response.body.Links;

            },
                response => {
                    // error callback
                });
        },
        validateEmail: function () {
            if (this.isValidLoginEmail()) {
                this.login.emailError = "";
            } else {
                this.login.emailError = "No valid email address";

            }
        },

        isValidLoginEmail: function () {
            var re = /^[a-z0-9._%+-]+@[a-z0-9.-]+\.[a-z]{2,3}$/;
            return re.test(this.login.email);
        },

        loginUser: function () {
            if (this.login.email && this.login.password && this.isValidLoginEmail()) {
                this.$http.post('/login', 'email=' + this.login.email + '&password=' + this.login.password).then(response => {
                    console.log(response.status);
                    if (response.ok) {
                        window.location.href = "/";

                    }
                },
                    response => {
                        this.login.password = "";
                        this.login.error = "The provided credentials are incorrect!";
                    });
            } else {
                this.login.error = "No fields can be empty";

            }
        },

        changePassword: function () {
            if (this.UserInfo.ChangePass.old && this.UserInfo.ChangePass.new) {
                this.$http.post('/changePassword', 'oldpassword=' + this.UserInfo.ChangePass.old +
                    '&newpassword=' + this.UserInfo.ChangePass.new).then(response => {
                        console.log(response.status);
                        if (response.ok) {
                            window.location.href = "/";

                        }
                    },
                    response => {
                        this.UserInfo.ChangePass.old = "";
                        this.UserInfo.ChangePass.new = "";
                        this.UserInfo.ChangePass.error = "The provided credentials are incorrect!";
                    });
            } else {
                this.UserInfo.ChangePass.error = "No fields can be empty";

            }
        },



        findBy: function (list, value, value2, column, col2) {
            return list.filter(function (item) {
                return item[column].includes(value) && item[col2].includes(value2)
            })
        },

        loadFlights: function () {
            this.$http.get('/getFlights').then(response => {

                // get body data
                this.FlightOverView.Flights = response.body.Flights;

            }, response => {
                // error callback
            });
            this.$http.get('/getFlightsBooked').then(response => {

                // get body data
                this.UserInfo.Flights = response.body.Flights;

            }, response => {
                // error callback
            });



        },

        loadFlight: function (Flight) {
            this.FlightOverView.FlightItem = Flight;
            this.FlightOverView.showModal = true;

        },

        book: function (flight) {
            console.log("erro " + flight.external_id);

            //Need to also to add to wich user this flight will be added to
            this.$http.post('/bookFlightWithId', 'id=' + flight.external_id).then(
                function () {
                    this.FlightOverView.showModal = false;
                    this.loadFlights();
                    this.loadLinks();
                },
                response => {
                    console.log("erro");
                });
        },
        loadUser: function () {
            this.$http.get('/getUserInfo').then(response => {

                // get body data
                this.UserInfo.UserInfo = response.body.Users;

            },
                response => {
                    // error callback
                });
        },
        adminLoadUsers: function () {
            this.$http.get('/adminGetUsers').then(response => {
                this.Admin.Users = response.body.Users;
            },
                response => {
                    // error callback
                });
        },
        sendNewMiles: function (user) {
            console.log(user.newMiles);
            console.log(user.email);
            this.$http.post('/adminSetNewAirMilesValue', 'email=' + user.email + '&airmiles=' + user.newMiles).then(response => {
                console.log("yeah");
                this.adminLoadUsers();
            },
                response => {
                    // error callback
                });
        }
    },
}));