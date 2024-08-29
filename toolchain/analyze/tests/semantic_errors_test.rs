use knot_analyze::{test::mocks::Mock, Error};
use kore::{assert_eq, str};
use lang::{ast, types, CanonicalId, NodeId};

#[test]
fn value_not_found() {
    let source = "const foo = bar;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (NodeId(0), Error::NotFound(str!("bar"))),
            (NodeId(1), Error::NotInferrable(vec![CanonicalId::mock(0)]))
        ])
    );
}

#[test]
fn type_not_found() {
    let source = "type foo = bar;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (NodeId(0), Error::NotFound(str!("bar"))),
            (NodeId(1), Error::NotInferrable(vec![CanonicalId::mock(0)]))
        ])
    );
}

#[test]
fn variant_not_found() {
    let source = "
enum foo { Integer(integer) | Empty }

const bar = foo.Other;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(3),
                Error::VariantNotFound(
                    CanonicalId::mock(2),
                    vec![str!("Integer"), str!("Empty")],
                    str!("Other")
                )
            ),
            (NodeId(4), Error::NotInferrable(vec![CanonicalId::mock(3)]))
        ])
    );
}

#[test]
fn declaration_not_found() {
    let source = "
module foo {
  const buzz = 123;
}

const bar = foo.fizz;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(5),
                Error::DeclarationNotFound(CanonicalId::mock(4), vec![str!("buzz")], str!("fizz"))
            ),
            (NodeId(6), Error::NotInferrable(vec![CanonicalId::mock(5)]))
        ])
    );
}

#[test]
fn not_indexable() {
    let source = "const foo = 123.bar;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(1),
                Error::NotIndexable(CanonicalId::mock(0), str!("bar"))
            ),
            (NodeId(2), Error::NotInferrable(vec![CanonicalId::mock(1)]))
        ])
    );
}

#[test]
fn property_not_found() {
    let source = "func foo(args: { bar: integer }) -> args.fizz";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::PropertyNotFound(CanonicalId::mock(3), vec![str!("bar")], str!("fizz"))
        ),])
    );
}

#[test]
fn duplicate_property() {
    let source = "
type foo = {
  bar: integer,
  bar: boolean
};";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(NodeId(2), Error::DuplicateProperty(str!("bar")))])
    );
}

#[test]
fn not_spreadable() {
    let source = "type foo = { ...integer };";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (NodeId(1), Error::NotSpreadable(CanonicalId::mock(0))),
            (NodeId(2), Error::NotInferrable(vec![CanonicalId::mock(1)]))
        ])
    );
}

#[test]
fn not_callable() {
    let source = "const foo = 123();";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (NodeId(1), Error::NotCallable(CanonicalId::mock(0))),
            (NodeId(2), Error::NotInferrable(vec![CanonicalId::mock(1)]))
        ])
    );
}

#[test]
fn untyped_parameter() {
    let source = "func foo(bar) -> nil;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(NodeId(0), Error::UntypedParameter(str!("bar")))])
    );
}

#[test]
fn default_value_rejected() {
    let source = "func foo(bar: integer = true) -> nil;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(2),
            Error::DefaultValueRejected(
                (CanonicalId::mock(0), types::Shape(types::Type::Integer)),
                (CanonicalId::mock(1), types::Shape(types::Type::Boolean))
            )
        )])
    );
}

#[test]
fn single_unexpected_argument() {
    let source = "
func foo -> nil;

const bar = foo(123);";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::UnexpectedArgument(CanonicalId::mock(3), 0)
        )])
    );
}

#[test]
fn multiple_unexpected_arguments() {
    let source = "
func foo -> nil;

const bar = foo(123, true);";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(5),
                Error::UnexpectedArgument(CanonicalId::mock(3), 0)
            ),
            (
                NodeId(5),
                Error::UnexpectedArgument(CanonicalId::mock(4), 0)
            )
        ])
    );
}

#[test]
fn single_missing_argument() {
    let source = "
func foo(a: integer) -> nil;

const bar = foo();";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(5),
            Error::MissingArgument(CanonicalId::mock(0), types::Shape(types::Type::Integer))
        )])
    );
}

#[test]
fn multiple_missing_arguments() {
    let source = "
func foo(a: integer, b: float) -> nil;

const bar = foo();";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(7),
                Error::MissingArgument(CanonicalId::mock(0), types::Shape(types::Type::Integer))
            ),
            (
                NodeId(7),
                Error::MissingArgument(CanonicalId::mock(2), types::Shape(types::Type::Float))
            )
        ])
    );
}

#[test]
fn single_rejected_argument() {
    let source = "
func foo(a: integer) -> nil;

const bar = foo(true);";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(6),
            Error::ArgumentRejected(
                (CanonicalId::mock(0), types::Shape(types::Type::Integer)),
                (CanonicalId::mock(5), types::Shape(types::Type::Boolean))
            )
        )])
    );
}

#[test]
fn multiple_rejected_arguments() {
    let source = "
func foo(a: integer, b: float) -> nil;

const bar = foo(true, nil);";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(9),
                Error::ArgumentRejected(
                    (CanonicalId::mock(0), types::Shape(types::Type::Integer)),
                    (CanonicalId::mock(7), types::Shape(types::Type::Boolean))
                )
            ),
            (
                NodeId(9),
                Error::ArgumentRejected(
                    (CanonicalId::mock(2), types::Shape(types::Type::Float)),
                    (CanonicalId::mock(8), types::Shape(types::Type::Nil))
                )
            )
        ])
    );
}

#[test]
fn single_unexpected_attribute() {
    let source = "
view foo -> nil;

const bar = <foo a=123 />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::UnexpectedAttribute(CanonicalId::mock(3), str!("a"))
        )])
    );
}

#[test]
fn multiple_unexpected_attributes() {
    let source = "
view foo -> nil;

const bar = <foo a=123 b=nil />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(6),
                Error::UnexpectedAttribute(CanonicalId::mock(3), str!("a"))
            ),
            (
                NodeId(6),
                Error::UnexpectedAttribute(CanonicalId::mock(5), str!("b"))
            )
        ])
    );
}

#[test]
fn single_missing_attribute() {
    let source = "
view foo { a: integer } -> nil;

const bar = <foo />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::MissingAttribute(
                CanonicalId::mock(0),
                str!("a"),
                types::Shape(types::Type::Integer)
            )
        )])
    );
}

#[test]
fn multiple_missing_attributes() {
    let source = "
view foo { a: integer, b: float } -> nil;

const bar = <foo />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(6),
                Error::MissingAttribute(
                    CanonicalId::mock(0),
                    str!("a"),
                    types::Shape(types::Type::Integer)
                )
            ),
            (
                NodeId(6),
                Error::MissingAttribute(
                    CanonicalId::mock(2),
                    str!("b"),
                    types::Shape(types::Type::Float)
                )
            )
        ])
    );
}

#[test]
fn single_rejected_attribute() {
    let source = "
view foo { a: integer } -> nil;

const bar = <foo a=true />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(6),
            Error::AttributeRejected(
                (
                    CanonicalId::mock(0),
                    str!("a"),
                    types::Shape(types::Type::Integer)
                ),
                (CanonicalId::mock(5), types::Shape(types::Type::Boolean))
            )
        )])
    );
}

#[test]
fn multiple_rejected_attributes() {
    let source = "
view foo { a: integer, b: float } -> nil;

const bar = <foo a=true b=nil />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(10),
                Error::AttributeRejected(
                    (
                        CanonicalId::mock(0),
                        str!("a"),
                        types::Shape(types::Type::Integer)
                    ),
                    (CanonicalId::mock(7), types::Shape(types::Type::Boolean))
                )
            ),
            (
                NodeId(10),
                Error::AttributeRejected(
                    (
                        CanonicalId::mock(2),
                        str!("b"),
                        types::Shape(types::Type::Float)
                    ),
                    (CanonicalId::mock(9), types::Shape(types::Type::Nil))
                )
            )
        ])
    );
}

#[test]
fn not_renderable() {
    let source = "
const foo = style {};

const bar = <>{foo}</>;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(3),
            Error::NotRenderable(CanonicalId::mock(2))
        ),])
    );
}

#[test]
fn invalid_component() {
    let source = "
const foo = 123;

const bar = <foo />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(2),
            Error::InvalidComponent(str!("foo"), types::Shape(types::Type::Integer))
        )])
    );
}

#[test]
fn component_typo() {
    let source = "
view foo -> nil;

const bar = <foo></food>;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(2),
            Error::ComponentTypo(str!("foo"), str!("food"))
        )])
    );
}

#[test]
fn binary_operation_not_supported() {
    let source = "const foo = true + false;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(2),
                Error::BinaryOperationNotSupported(
                    ast::BinaryOperator::Add,
                    (CanonicalId::mock(0), None),
                    (CanonicalId::mock(1), None)
                )
            ),
            (NodeId(3), Error::NotInferrable(vec![CanonicalId::mock(2)]))
        ])
    );
}

#[test]
fn unary_operation_not_supported() {
    let source = "const foo = !123;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(1),
            Error::UnaryOperationNotSupported(
                ast::UnaryOperator::Not,
                (
                    CanonicalId::mock(0),
                    Some(types::Shape(types::Type::Integer))
                )
            )
        ),])
    );
}
