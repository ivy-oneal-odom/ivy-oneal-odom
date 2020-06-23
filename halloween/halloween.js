$(document).ready(function(){

	var display = $("#countdown");

	function getCountdown(){
		var date = new Date();
		var currentDate = date.getDate();
		var currentMonth = date.getMonth();
		var currentYear = date.getYear() + 1900;

		var currentEpoch = new Date().getTime();
		var nextHalloween;

		if(currentDate == 31 && currentMonth == 9){	//It is halloween!
			display.text("It is halloween! Be gay! Do crimes!");
		}else{

			//Find next halloween
			if(currentMonth < 9 || (currentMonth == 9 && currentDate < 31)){	//It isn't halloween yet
				nextHalloween = "Oct 31, " + currentYear;
			}else{
				nextHalloween = "Oct 31, " + (currentYear + 1);
			}

			//Seconds until next Halloween
			var distance = new Date(nextHalloween).getTime() - currentEpoch;

			//Calculations (credit to w3schools)
			var days = Math.floor(distance / (1000 * 60 * 60 * 24));
			var hours = Math.floor((distance % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60));
			var minutes = Math.floor((distance % (1000 * 60 * 60)) / (1000 * 60));
			var seconds = Math.floor((distance % (1000 * 60)) / 1000);

			//Display it babey
			display.text(days + " Days, " + hours + " Hours, " + minutes + " Minutes, and " + seconds + " Seconds");
		}
		setTimeout(function(){
			getCountdown();
		}, 1000);
	}

	getCountdown();
});