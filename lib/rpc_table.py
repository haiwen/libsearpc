"""
Define RPC functions needed to generate
"""

# [ <ret-type>, [<arg_types>] ]
func_table = [
    [ "int", [] ],
    [ "int", ["string"] ],
    [ "int", ["string", "int"] ],
    [ "int", ["string", "string"] ],
    [ "int", ["string", "string", "string"] ],
    [ "string", [] ],
    [ "string", ["string"] ],
    [ "string", ["string", "int"] ],
    [ "string", ["string", "string"] ],
    [ "string", ["string", "string", "string"] ],
    [ "string", ["string", "string", "string", "string"] ],
    [ "objlist", [] ],
    [ "objlist", ["string"] ],
    [ "objlist", ["int", "int"] ],
    [ "objlist", ["string", "int"] ],
    [ "objlist", ["string", "int", "int"] ],
    [ "objlist", ["string", "string", "int"] ],
    [ "object", [] ],
    [ "object", ["string"] ],
]
