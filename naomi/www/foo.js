// menu.js

function hoverGalleryImg(obj) {
	
	var gallery = obj.closest(".gallery");
	var galleryDesc = gallery.querySelector(".gallery_desc");
	galleryDesc.innerHTML = obj.alt + ' being an absolute ICON';
}

function houterGalleryImg(obj) {
	var gallery = obj.closest(".gallery");
	var galleryDesc = gallery.querySelector(".gallery_desc");
	galleryDesc.innerHTML = obj.alt + ' being an absolute icon';
}

function houterThisIs(obj) {
	// var menuContainer = obj.querySelector(".menu-container")
	obj.innerHTML = `
	<div id="this-is" onmouseover="hoverThisIs(this)">
		This is
	</div>The hall of fame
	`;
}


function hoverThisIs(obj) {
	// var menuContainer = obj.parentElement;
	// var menu = menuContainer.querySelector(".menu");
	// var menu = document.getElementsByClassName('menu');
	var menu = obj.closest('menu');

	menu.innerHTML = `
	<ul id="menu>
	<li class="menu">
			<a
			href="denholm.html">
				Douglas
		</a>
	</li>
	<li class="menu">
		<a
			class="menu"
			href="dwight.html">
				Dwight
		</a>
	</li>
	<li class="menu">
		<a
			class="menu"
			href="lucille.html">
				Lucille
		</a>
	</li>
	<li class="menu">
		<a
			class="menu"
			href="roy.html">
				Roy
		</a>
	</li>
	<li class="menu">
		<a
			class="menu"
			href="hall-of-fame.html">
				hall of fame
		</a>
	</li>
	<li class="menu">
		<a
			class="menu"
			href="upload.html">
				upload
		</a>
	</li>
	</ul>
`;
}

document.addEventListener("DOMContentLoaded", function () {
	
	const footer = document.getElementById('footer');
	
	footer.innerHTML = `
			<li class="footer">
				<a href="index.html">
					Yaaasss bitch
				</a>
			</li>
			<li class="footer">
				<a href="debug.debug">
					Debug info
				</a>
			</li>
			<div id="shutup">
				<li class="footer">
					<a href="shutup.html">shut up
					</a></li> </div>
	`;

	// const menu = document.getElementById('menu');

    // menu.innerHTML = `
	// 	<div id="this-is">
	// 		This is...
	// 	</div>
	// 	<li class="menu">
	// 			<a
	// 			href="denholm.html">
	// 				Douglas
	// 		</a>
	// 	</li>
	// 	<li class="menu">
	// 		<a
	// 			class="menu"
	// 			href="dwight.html">
	// 				Dwight
	// 		</a>
	// 	</li>
	// 	<li class="menu">
	// 		<a
	// 			class="menu"
	// 			href="lucille.html">
	// 				Lucille
	// 		</a>
	// 	</li>
	// 	<li class="menu">
	// 		<a
	// 			class="menu"
	// 			href="roy.html">
	// 				Roy
	// 		</a>
	// 	</li>
	// 	<li class="menu">
	// 		<a
	// 			class="menu"
	// 			href="hall-of-fame.html">
	// 				hall of fame
	// 		</a>
	// 	</li>
	// 	<li class="menu">
	// 		<a
	// 			class="menu"
	// 			href="upload.html">
	// 				upload
	// 		</a>
	// 	</li>
    // `;
});
