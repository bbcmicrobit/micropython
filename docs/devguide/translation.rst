.. _dev-translate:

============
Translations
============

The Python community is a diverse and cosmopolitan place. Our documentation
should be available for users in many different languages and for many different
locales.

This document explains how to go about translating this documentation.

Read The Docs
-------------

Currently we host our documentation on http://readthedocs.org. We use Sphinx to
make the documentation happen.

The relevant documentation for localisation in ReadTheDocs is here:

https://docs.readthedocs.io/en/latest/localization.html#project-with-multiple-translations

Put simply, each new translation is a separate project, presumably a Sphinx
based repository on GitHub, that contains the translated resources and is linked
to ReadTheDocs in the usual way - making sure its locale is set  appropriately.

It's possible for this "core" project to link to these other projects as
"translations of" this one.

Read the Docs automatically figures out the relationships and displays the
appropriate links between all the related translation projects.

I want to Translate!
--------------------

Great stuff! Thank you (in advance) for your support!

* Make a new Sphinx project that mirrors the structure of the ``docs`` directory in this project. Call the project something obvious like: ``microbit-micropython-es`` (for the Spanish translation).
* Register the project on ReadTheDocs (see: https://docs.readthedocs.io/en/latest/getting_started.html)
* Create a new issue pointing to the new ReadTheDocs project that forms the new translation. We do this so others have visibility of what's being done. Submit the issue here: https://github.com/bbcmicrobit/micropython/issues/new
* After a quick check and review that things look in order we'll link you as a translation!

That's it! :-)
