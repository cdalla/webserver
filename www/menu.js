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

document.addEventListener("DOMContentLoaded", function () {
	
	const footer = document.getElementById('footer');
	
	footer.innerHTML = `
			<li class="footer">
				<a href="index.html">
					Back 2 home
				</a>
			</li>
	`;

	const menu = document.getElementById('menu');

    menu.innerHTML = `
		<div id="this-is">
			This is...
		</div>
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
		<li class="menu">
			<a
				class="menu"
				href="upload2.html">
					da bonus upload
			</a>
		</li>
    `;
});
