#-----------------------------------------------------------------------------
# luna2d DeployTool
# This is part of luna2d engine
# Copyright 2014-2016 Stepan Prokofjev
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#-----------------------------------------------------------------------------

import constants_wp
import utils
import re

def do_update(args, config, project_name):
	orientation = config["orientation"] if "orientation" in config else "landscape"

	# Update "Package.appxmanifest"
	utils.substitute_file_regexp(args.project_path + "/" + project_name + "/Package.appxmanifest",
		constants_wp.ORIENTATION_REGEXP,
		constants_wp.ORIENTATION_LANDSCAPE if orientation == "landscape" else constants_wp.ORIENTATION_PORTRAIT)