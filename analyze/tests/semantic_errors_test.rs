use knot_analyze::{AmbientMap, Context, Error, ModuleMap, Result, TypeMap};
use kore::{assert_eq, str};
use lang::{ast, CanonicalId, NodeId};

#[derive(Default)]
struct Mock {
    modules: ModuleMap,
    ambient: AmbientMap,
}

impl Mock {
    #[allow(clippy::unwrap_in_result)]
    fn parse_and_analyze(self, source: &str) -> Result<(ast::typed::Program, TypeMap)> {
        let ctx = Context::mock(&self.modules, &self.ambient);
        let (raw, _) = parse::program::parse(source).ok().unwrap();

        knot_analyze::analyze(&ctx, raw)
    }
}

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
enum foo = Integer(integer) | Empty;

const bar = foo.Other;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(3),
                Error::VariantNotFound(CanonicalId::mock(2), str!("Other"))
            ),
            (NodeId(4), Error::NotInferrable(vec![CanonicalId::mock(3)]))
        ])
    );
}

#[test]
fn declaration_not_found() {
    let source = "
module foo {}

const bar = foo.fizz;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(3),
                Error::DeclarationNotFound(CanonicalId::mock(2), str!("fizz"))
            ),
            (NodeId(4), Error::NotInferrable(vec![CanonicalId::mock(3)]))
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
        Err(vec![(NodeId(0), Error::UntypedParameter)])
    );
}

#[test]
fn default_value_rejected() {
    let source = "func foo(bar: integer = true) -> nil;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(2),
            Error::DefaultValueRejected(CanonicalId::mock(1))
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
            Error::UnexpectedArgument(CanonicalId::mock(3))
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
            (NodeId(5), Error::UnexpectedArgument(CanonicalId::mock(3))),
            (NodeId(5), Error::UnexpectedArgument(CanonicalId::mock(4)))
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
            Error::MissingArgument(CanonicalId::mock(0))
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
            (NodeId(7), Error::MissingArgument(CanonicalId::mock(0))),
            (NodeId(7), Error::MissingArgument(CanonicalId::mock(2)))
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
            Error::ArgumentRejected(CanonicalId::mock(0), CanonicalId::mock(5))
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
                Error::ArgumentRejected(CanonicalId::mock(0), CanonicalId::mock(7))
            ),
            (
                NodeId(9),
                Error::ArgumentRejected(CanonicalId::mock(2), CanonicalId::mock(8))
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
        Err(vec![(NodeId(4), Error::UnexpectedAttribute(str!("a")))])
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
            (NodeId(6), Error::UnexpectedAttribute(str!("a"))),
            (NodeId(6), Error::UnexpectedAttribute(str!("b")))
        ])
    );
}

#[test]
fn single_missing_attribute() {
    let source = "
view foo(a: integer) -> nil;

const bar = <foo />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(4),
            Error::MissingAttribute(CanonicalId::mock(0))
        )])
    );
}

#[test]
fn multiple_missing_attributes() {
    let source = "
view foo(a: integer, b: float) -> nil;

const bar = <foo />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (NodeId(6), Error::MissingAttribute(CanonicalId::mock(0))),
            (NodeId(6), Error::MissingAttribute(CanonicalId::mock(2)))
        ])
    );
}

#[test]
fn single_rejected_attribute() {
    let source = "
view foo(a: integer) -> nil;

const bar = <foo a=true />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![(
            NodeId(6),
            Error::AttributeRejected(CanonicalId::mock(0), CanonicalId::mock(5))
        )])
    );
}

#[test]
fn multiple_rejected_attributes() {
    let source = "
view foo(a: integer, b: float) -> nil;

const bar = <foo a=true b=nil />;";

    assert_eq!(
        Mock::default().parse_and_analyze(source),
        Err(vec![
            (
                NodeId(10),
                Error::AttributeRejected(CanonicalId::mock(0), CanonicalId::mock(7))
            ),
            (
                NodeId(10),
                Error::AttributeRejected(CanonicalId::mock(2), CanonicalId::mock(9))
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
        Err(vec![(NodeId(2), Error::InvalidComponent(str!("foo")))])
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
                    CanonicalId::mock(0),
                    CanonicalId::mock(1)
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
            Error::UnaryOperationNotSupported(ast::UnaryOperator::Not, CanonicalId::mock(0),)
        ),])
    );
}
