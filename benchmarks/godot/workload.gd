extends SceneTree

func dispatch(count: int) -> int:
    var total: int = 0
    for i in count:
        match i & 3:
            0: total += i
            1: total -= i
            2: total += i * 3
            3: total -= i * 2
    return total

func arrays(count: int) -> int:
    var values := PackedInt32Array()
    values.resize(count)
    for i in count:
        values[i] = (i * 17) & 65535
    var total: int = 0
    for i in count:
        total += values[i]
    return total

func strings(count: int) -> int:
    var total: int = 0
    for i in count:
        var text := "compiler:%d:%d" % [i & 255, i & 15]
        total += text.length() + text.find(":")
    return total

func geometry(count: int) -> int:
    var total: int = 0
    var axis := Vector3(2, 3, 5)
    for i in count:
        var point := Vector3(i & 31, i & 15, i & 7)
        var product := point.cross(axis)
        total += int(product.x + product.y + product.z)
    return total

func scene(count: int) -> int:
    var node := Node2D.new()
    var total: int = 0
    for i in count:
        node.position = Vector2(i & 31, i & 15)
        node.rotation = 0.0
        var point := node.transform * Vector2(2, 3)
        total += int(point.x + point.y)
    node.free()
    return total

func _initialize():
    var args := OS.get_cmdline_user_args()
    if args.size() != 3 or not args[1].is_valid_int() or not args[2].is_valid_int():
        printerr("Expected workload, iterations and repeats")
        quit(1)
        return
    var name: String = args[0]
    var count := int(args[1])
    var repeats := int(args[2])
    if name not in ["dispatch", "arrays", "strings", "geometry", "scene"] or count <= 0 or repeats <= 0:
        quit(2)
        return
    var checksum: int = 0
    var started := Time.get_ticks_usec()
    for i in repeats:
        checksum += call(name, count)
    var elapsed := Time.get_ticks_usec() - started
    print("CPPGM_GODOT ", JSON.stringify({"workload": name, "iterations": count,
        "repeats": repeats, "checksum": checksum, "elapsed_usec": elapsed}))
    quit(0)
