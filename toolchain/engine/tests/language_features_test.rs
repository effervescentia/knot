mod common;

use common::assert_valid;

#[test]
fn static_enum_variant_instance() {
    assert_valid(
        "enum Foo { Bar }
func noop(input: Foo) -> nil;
const result = noop(Foo.Bar);",
    );
}

#[test]
fn dynamic_enum_variant_instance() {
    assert_valid(
        "enum Foo { Bar(integer) }
func noop(input: Foo) -> nil;
const result = noop(Foo.Bar(123));",
    );
}

#[test]
fn nested_enum_variant_instance() {
    assert_valid(
        "enum Foo { Bar }
enum Fizz { Buzz(Foo) }
func noop(input: Fizz) -> nil;
const result = noop(Fizz.Buzz(Foo.Bar));",
    );
}
