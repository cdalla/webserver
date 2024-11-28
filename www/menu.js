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
				<a href="html/index.html">
					Yaaasss bitch
				</a>
			</li>
			<li class="footer">
				<a href="html/debug.debug">
					Debug info
				</a>
			</li>
			<div id="shutup">
				<li class="footer">
					<a href="html/shutup.html">shut up
					</a></li> </div>
	`;

	const menu = document.getElementById('menu');

    menu.innerHTML = `
		<div id="this-is">
			This is...
		</div>
		<li class="menu">
				<a
				href="html/denholm.html">
					Douglas
			</a>
		</li>
		<li class="menu">
			<a
				class="menu"
				href="html/dwight.html">
					Dwight
			</a>
		</li>
		<li class="menu">
			<a
				class="menu"
				href="html/lucille.html">
					Lucille
			</a>
		</li>
		<li class="menu">
			<a
				class="menu"
				href="html/roy.html">
					Roy
			</a>
		</li>
		<li class="menu">
			<a
				class="menu"
				href="html/hall-of-fame.html">
					hall of fame
			</a>
		</li>
		<li class="menu">
			<a
				class="menu"
				href="html/upload.html">
					upload
			</a>
		</li>
    `;
});
