# cppgm.tests

`undefined/paN` keeps inputs whose outcome the course leaves unspecified
(a byte-order mark, a header name no grammar accepts, a raw string with
characters outside the basic source character set).  No lane runs them;
they document what an assignment does not promise.

Every test that an assignment runs lives under that assignment's own
`paN/tests/` directory.  See `TESTING_AND_REFERENCES.md` at the repository
root for the lanes and the reference policy.
