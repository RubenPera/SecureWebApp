/**
 * Created by ruben on 5/31/17.
 */

Vue.component('modal', {
    template: '#flightOverView-modal-template'
})

var masterPage = new Vue({
    el: '#masterPage',
    data: {
        Links: [],
        message: 'Hello Vue!',
        login: {
            email: "user2@teamalfa.nl",
            password: "",
            error: "",
            emailError: "",
        },
        FlightOverView: {
        FlightItem: {
            id: 0,
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

            Flights: [],
            query: '',
            query2: '',
        },


    },
    computed: {
        tableFilter: function () {
            return this.findBy(this.FlightOverView.Flights, this.FlightOverView.query, this.FlightOverView.query2, 'flight_source', 'flight_destination')
        }

    },
    mounted: function () {
        this.loadLinks();
        this.validateEmail();
        this.loadFlights();
        this.loadUser();
    },
    methods: {
        loadLinks: function () {
            var _this = this;

            this.$http.get('/getLinks').then(response => {

                // get body data
                this.Links = response.body.Links;

        },
            response =>
            {
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
            if (this.login.email && this.login.password) {
                this.$http.post('/login', 'email=' + this.login.email + '&password=' + this.login.password).then(response => {
                    console.log(response.status);
                if (response.ok) {
                    window.location.href = "/";

                }
            },
                response =>
                {
                    this.login.password = "";
                    this.login.error = "The provided credentials are incorrect!";
                });
            } else {
                this.login.error = "No fields can be empty";

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

            //Need to also to add to wich user this flight will be added to
            this.$http.post('/bookFlight', "id="+ flight.id).then(
                function () {
                    this.FlightOverView.showModal = false;
                    this.loadFlights();
                    this.loadLinks();
                });
        },
        loadUser: function () {
            this.$http.get('/getUserInfo?id=1').then(response => {

                // get body data
                this.UserInfo.UserInfo = response.body.Users[0];

        },
            response =>
            {
                // error callback
            });
        },
    },
});