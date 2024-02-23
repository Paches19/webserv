/*
Import handlebars
Create template for the modal
On click, the modal appears
On click of X or click off the modal, modal dissapears

Template:
<img src="images/img_${firstName}"

Algorithm:
- On click, grab the target's img tag, grab their name from alt attribute
- Create template using data and

*/

$().ready(function() {
  let modalTemplate = Handlebars.compile($('#modal-script').html());

  let randomText = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";
  let $memberLinks = $('div#team a');
  let $team = $('#team');

  $memberLinks.on('click', (e) => {
    e.preventDefault(); // e is img tag
    e.stopPropagation();
    let name = e.target.alt;
    let img = e.target;
    let context = {
      'name': name,
      'imageLink': img.src,
      'randomText': randomText,
    }
    let modal = modalTemplate(context);
    $(modal).insertBefore("#team");
    $(document.body).addClass('fade');

    $('#modal a').on('click', (ev) => {
      ev.preventDefault();
      $('#modal').remove();
      $(document.body).removeClass('fade');
    });

    $(document).on('click', (event) => {
      let modal = document.getElementById('modal');
      if (modal) {
        if (!($.contains(modal, event.target))) {
          $('#modal').remove();
          $(document.body).removeClass('fade');
        }
      }
    });
  });

})
