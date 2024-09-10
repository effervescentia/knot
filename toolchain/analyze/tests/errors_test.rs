use knot_analyze::{test::mocks::Mock, Error};
use kore::{assert_eq, str};
use lang::{ast, types, CanonicalId, NodeId};

#[test]
fn e401_value_not_found() {
    let source = include_str!("../../../examples/invalid/401_not_found/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (NodeId(0), Error::NotFound(str!("BAR"))),
            (NodeId(1), Error::NotInferrable(vec![CanonicalId::mock(0)]))
        ])
    );
}

#[test]
fn e401_type_not_found() {
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
fn e402_variant_not_found() {
    let source = include_str!("../../../examples/invalid/402_variant_not_found/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(2),
                Error::VariantNotFound(
                    CanonicalId::mock(1),
                    vec![str!("Fizz"), str!("Buzz")],
                    str!("Bar")
                )
            ),
            (NodeId(3), Error::NotInferrable(vec![CanonicalId::mock(2)]))
        ])
    );
}

#[test]
fn e403_declaration_not_found() {
    let source = include_str!("../../../examples/invalid/403_declaration_not_found/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(5),
                Error::DeclarationNotFound(CanonicalId::mock(4), vec![str!("buzz")], str!("bar"))
            ),
            (NodeId(6), Error::NotInferrable(vec![CanonicalId::mock(5)]))
        ])
    );
}

#[test]
fn e404_not_indexable() {
    let source = include_str!("../../../examples/invalid/404_not_indexable/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(1),
                Error::NotIndexable(CanonicalId::mock(0), str!("foo"))
            ),
            (NodeId(2), Error::NotInferrable(vec![CanonicalId::mock(1)]))
        ])
    );
}

#[test]
fn e405_property_not_found() {
    let source = include_str!("../../../examples/invalid/405_property_not_found/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::PropertyNotFound(CanonicalId::mock(3), vec![str!("bar")], str!("fizz"))
        ),])
    );
}

#[test]
fn e406_duplicate_property() {
    let source = include_str!("../../../examples/invalid/406_duplicate_property/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(NodeId(2), Error::DuplicateProperty(str!("bar")))])
    );
}

#[test]
fn e407_not_spreadable() {
    let source = include_str!("../../../examples/invalid/407_not_spreadable/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (NodeId(1), Error::NotSpreadable(CanonicalId::mock(0))),
            (NodeId(2), Error::NotInferrable(vec![CanonicalId::mock(1)]))
        ])
    );
}

#[test]
fn e408_untyped_parameter() {
    let source = include_str!("../../../examples/invalid/408_untyped_parameter/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(NodeId(0), Error::UntypedParameter(str!("bar")))])
    );
}

#[test]
fn e409_default_value_rejected() {
    let source = include_str!("../../../examples/invalid/409_default_value_rejected/src/main.kn");

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
fn e410_not_callable() {
    let source = include_str!("../../../examples/invalid/410_not_callable/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (NodeId(1), Error::NotCallable(CanonicalId::mock(0))),
            (NodeId(2), Error::NotInferrable(vec![CanonicalId::mock(1)]))
        ])
    );
}

#[test]
fn e411_single_unexpected_argument() {
    let source = include_str!("../../../examples/invalid/411_unexpected_argument/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::UnexpectedArgument(CanonicalId::mock(3), 0)
        )])
    );
}

#[test]
fn e411_multiple_unexpected_arguments() {
    let source = "
func foo -> nil;

const FOO = foo(123, true);";

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
fn e412_single_missing_argument() {
    let source = include_str!("../../../examples/invalid/412_missing_argument/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(5),
            Error::MissingArgument(CanonicalId::mock(0), types::Shape(types::Type::Integer))
        )])
    );
}

#[test]
fn e412_multiple_missing_arguments() {
    let source = "
func foo(bar: integer, fizz: float) -> nil;

const FOO = foo();";

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
fn e413_single_rejected_argument() {
    let source = include_str!("../../../examples/invalid/413_argument_rejected/src/main.kn");

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
fn e413_multiple_rejected_arguments() {
    let source = "
func foo(bar: integer, fizz: float) -> nil;

const FOO = foo(true, nil);";

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
fn e414_not_renderable() {
    let source = include_str!("../../../examples/invalid/414_not_renderable/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(1),
            Error::NotRenderable(CanonicalId::mock(0))
        ),])
    );
}

#[test]
fn e415_invalid_component() {
    let source = include_str!("../../../examples/invalid/415_invalid_component/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(2),
            Error::InvalidComponent(str!("Foo"), types::Shape(types::Type::Integer))
        )])
    );
}

#[test]
fn e416_component_typo() {
    let source = include_str!("../../../examples/invalid/416_component_typo/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(2),
            Error::ComponentTypo(str!("Foo"), str!("Bar"))
        )])
    );
}

#[test]
fn e418_single_unexpected_attribute() {
    let source = include_str!("../../../examples/invalid/418_unexpected_attribute/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::UnexpectedAttribute(CanonicalId::mock(3), str!("bar"))
        )])
    );
}

#[test]
fn e418_multiple_unexpected_attributes() {
    let source = "
view Foo -> nil;

const FOO = <Foo bar=123 fizz=nil />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(6),
                Error::UnexpectedAttribute(CanonicalId::mock(3), str!("bar"))
            ),
            (
                NodeId(6),
                Error::UnexpectedAttribute(CanonicalId::mock(5), str!("fizz"))
            )
        ])
    );
}

#[test]
fn e419_single_missing_attribute() {
    let source = include_str!("../../../examples/invalid/419_missing_attribute/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::MissingAttribute(
                CanonicalId::mock(0),
                str!("bar"),
                types::Shape(types::Type::Integer)
            )
        )])
    );
}

#[test]
fn e419_multiple_missing_attributes() {
    let source = "
view foo { bar: integer, fizz: float } -> nil;

const FOO = <foo />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(6),
                Error::MissingAttribute(
                    CanonicalId::mock(0),
                    str!("bar"),
                    types::Shape(types::Type::Integer)
                )
            ),
            (
                NodeId(6),
                Error::MissingAttribute(
                    CanonicalId::mock(2),
                    str!("fizz"),
                    types::Shape(types::Type::Float)
                )
            )
        ])
    );
}

#[test]
fn e420_single_rejected_attribute() {
    let source = include_str!("../../../examples/invalid/420_attribute_rejected/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(6),
            Error::AttributeRejected(
                (
                    CanonicalId::mock(0),
                    str!("bar"),
                    types::Shape(types::Type::Integer)
                ),
                (CanonicalId::mock(5), types::Shape(types::Type::Boolean))
            )
        )])
    );
}

#[test]
fn e420_multiple_rejected_attributes() {
    let source = "
view Foo { bar: integer, fizz: float } -> nil;

const FOO = <Foo bar=true fizz=nil />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(10),
                Error::AttributeRejected(
                    (
                        CanonicalId::mock(0),
                        str!("bar"),
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
                        str!("fizz"),
                        types::Shape(types::Type::Float)
                    ),
                    (CanonicalId::mock(9), types::Shape(types::Type::Nil))
                )
            )
        ])
    );
}

#[test]
fn e421_binary_operation_not_supported() {
    let source =
        include_str!("../../../examples/invalid/421_binary_operation_not_supported/src/main.kn");

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
fn e422_unary_operation_not_supported() {
    let source =
        include_str!("../../../examples/invalid/422_unary_operation_not_supported/src/main.kn");

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(1),
            Error::UnaryOperationNotSupported(
                ast::UnaryOperator::Not,
                (
                    CanonicalId::mock(0),
                    Some(types::Shape(types::Type::String))
                )
            )
        ),])
    );
}
