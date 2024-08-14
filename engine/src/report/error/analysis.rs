use crate::report::example;

use super::{code::ToCode, CodeFrame, Display, ErrorCode, ErrorDisplay};
use kore::{
    color::{Colorize, Highlight},
    format::Indented,
    str,
};
use std::collections::HashMap;

impl ToCode for analyze::Error {
    fn to_code(&self) -> ErrorCode {
        match self {
            Self::NotInferrable(_) => ErrorCode::NOT_INFERRABLE,
            Self::NotFound(_) => ErrorCode::NOT_FOUND,
            Self::VariantNotFound(..) => ErrorCode::VARIANT_NOT_FOUND,
            Self::DeclarationNotFound(..) => ErrorCode::DECLARATION_NOT_FOUND,
            Self::NotIndexable(..) => ErrorCode::NOT_INDEXABLE,
            Self::PropertyNotFound(..) => ErrorCode::PROPERTY_NOT_FOUND,
            Self::DuplicateProperty(_) => ErrorCode::DUPLICATE_PROPERTY,
            Self::NotSpreadable(_) => ErrorCode::NOT_SPREADABLE,
            Self::UntypedParameter(_) => ErrorCode::UNTYPED_PARAMETER,
            Self::DefaultValueRejected(_) => ErrorCode::DEFAULT_VALUE_REJECTED,
            Self::NotCallable(_) => ErrorCode::NOT_CALLABLE,
            Self::UnexpectedArgument(_) => ErrorCode::UNEXPECTED_ARGUMENT,
            Self::MissingArgument(_) => ErrorCode::MISSING_ARGUMENT,
            Self::ArgumentRejected(..) => ErrorCode::ARGUMENT_REJECTED,
            Self::NotRenderable(_) => ErrorCode::NOT_RENDERABLE,
            Self::InvalidComponent(_) => ErrorCode::INVALID_COMPONENT,
            Self::ComponentTypo(..) => ErrorCode::COMPONENT_TYPO,
            Self::InvalidAttributes(_) => ErrorCode::INVALID_ATTRIBUTES,
            Self::UnexpectedAttribute(_) => ErrorCode::UNEXPECTED_ATTRIBUTE,
            Self::MissingAttribute(_) => ErrorCode::MISSING_ATTRIBUTE,
            Self::AttributeRejected(..) => ErrorCode::ATTRIBUTE_REJECTED,
            Self::BinaryOperationNotSupported(..) => ErrorCode::BINARY_OPERATION_NOT_SUPPORTED,
            Self::UnaryOperationNotSupported(..) => ErrorCode::UNARY_OPERATION_NOT_SUPPORTED,
            Self::UnexpectedKind(..) => ErrorCode::UNEXPECTED_KIND,
        }
    }
}

impl<'a> Display<'a> for analyze::Error {
    type Context = (
        &'a lang::CanonicalId,
        &'a str,
        &'a HashMap<lang::NamespaceId, (crate::Link, String)>,
        &'a HashMap<lang::CanonicalId, lang::Range>,
    );

    fn display(&'a self, (id, root_dir, modules, nodes): Self::Context) -> ErrorDisplay<'a> {
        let bind = |title, description, target, suggestion, examples| {
            let module = modules.get(&id.0);
            let range = nodes.get(id);

            let code_frame = match (target, module, range) {
                (Some((highlight, error)), Some((link, text)), _) => {
                    Some(CodeFrame::DualTarget {
                        root_dir,
                        link,
                        source: text,
                        highlight,
                        error,
                        color: true
                    })
                }
                (None, Some((link, text)), Some(range)) => {
                    Some(CodeFrame::SingleTarget {
                        root_dir,
                        link,
                        source: text,
                        error: *range,
                        color: true
                    })
                }
                _ => None,
            };

            ErrorDisplay {
                code: self.to_code(),
                title,
                description,
                suggestion,
                examples,
                code_frame,
            }
        };

        let temp = |title, description| bind(title, description, None, None, vec![]);

        match self {
            Self::NotInferrable(_) => temp(
                "Not Inferrable",
                str!("the type of this expression could not be inferred from other types"),
            ),

            Self::NotFound(name) => bind(
                "Not Found",
                format!(
                    "This expression references a variable named {} which does not exist.",
                    name.error()
                ),
                None,
                Some(str!("Check the spelling of the variable name or declare a new variable.")),
                vec![example::local_variables(name)]
            ),

            // TODO: use a two-target code frame
            Self::VariantNotFound(_, variant) => temp(
                "Variant Not Found",
                format!(
                    "{} does not have a variant named {}.",
                    "This enumerated type".highlight(),
                    variant.error()
                ),
            ),

            // TODO: use a two-target code frame
            Self::DeclarationNotFound(_, name) => temp(
                "Declaration Not Found",
                format!(
                    "This module does not contain a declaration named {}.",
                    name.error()
                ),
            ),

            // TODO: use a two-target code frame
            Self::NotIndexable(_, name) => temp(
                "Not Indexable",
                // TODO: fix this error message
                format!(
                    "The property {} cannot be accessed because this type of value does not named",
                    name.error()
                ), // format!("the property {} cannot be accessed because this type of value does not support named properties", name.error())
            ),

            // TODO: use a two-target code frame
            Self::PropertyNotFound(_, name) => temp(
                "Property Not Found",
                format!("a property with the name {} was not found", name.error()),
            ),

            Self::DuplicateProperty(name) => bind(
                "Duplicate Property",
                format!(
                    "This expression contains two or more properties named {}. All property names must be unique.",
                    name.error()
                ),
                None,
                Some(
                    format!("Remove or rename any repeated {} properties to avoid conflict.", name.highlight())
                ),
                vec![example::object_properties(&format!("{}_0", name), &format!("{}_1", name))]
            ),

            // TODO: use a two-target code frame
            Self::NotSpreadable(_) => temp(
                "Not Spreadable",
                str!("only object-like types can be spread"),
            ),

            Self::UntypedParameter(name) => bind(
                "Untyped Parameter",
                format!("The parameter {} is missing a type annotation.", name.error()),
                None,
                Some(
                    format!("Add an annotation describing the type of the {} parameter.", name.highlight())
                ),
                vec![example::parameter_types()]
            ),

            // TODO: use a two-target code frame
            Self::DefaultValueRejected(_) => temp(
                "Default Value Rejected",
                "The type of the default value does not match the annotated type of the parameter."
                    .to_string(),
            ),

            // TODO: use a two-target code frame
            Self::NotCallable(_) => temp(
                "Not Callable",
                str!("this expression is not a function and cannot be called"),
                
            ),

            // TODO: use a two-target code frame
            Self::UnexpectedArgument(_) => temp(
                "Unexpected Argument",
                str!("this function call expects fewer arguments than were provided"),
                
            ),

            Self::MissingArgument(_) => temp(
                "Missing Argument",
                str!("this function call expects an additional argument that was not provided"),
                
            ),

            Self::ArgumentRejected(_, _) => temp(
                "Argument Rejected",
                str!(
                    "this argument did not match the expected type based on the function signature"
                ),
                
            ),

            Self::NotRenderable(_) => temp(
                "Not Renderable",
                str!("this expression is not a component and cannot be rendered"),
                
            ),

            Self::InvalidComponent(name) => temp(
                "Invalid Component",
                format!(
                    "the variable {} does not reference a valid component type",
                    name.error()
                ),
            ),

            Self::ComponentTypo(start_tag, end_tag) => bind(
                "Component Typo",
                format!(
                    "The start {} and end {} tags of this component do not match.",
                    format!("<{start_tag}>").error(),
                    format!("</{end_tag}>").error(),
                ),
                None,
                Some(str!("Change the end tag to match the start tag.")),
                vec![example::open_component(start_tag)]
            ),

            Self::InvalidAttributes(_) => temp("Invalid Attributes", format!("")),

            Self::UnexpectedAttribute(name) => temp(
                "Unexpected Attribute",
                format!(
                    "this component does not accept an attribute named {}",
                    name.error()
                ),
            ),

            Self::MissingAttribute(_) => temp(
                "Missing Attribute",
                str!("this component expected an attribute that was not provided"),
            ),

            Self::AttributeRejected(_, _) => temp(
                "Attribute Rejected",
                "this attribute did not match the expected type based on the component signature"
                    .to_owned(),
            ),

            Self::BinaryOperationNotSupported(op, _, _) => temp(
                "Binary Operation Not Supported",
                format!(
                    "the operator {} cannot be applied to the arguments provided",
                    op.to_string().highlight()
                ),
            ),

            Self::UnaryOperationNotSupported(op, _) => temp(
                "Unary Operation Not Supported",
                format!(
                    "the operator {} cannot be applied to the argument provided",
                    op.to_string().highlight()
                ),
            ),

            Self::UnexpectedKind(_, kind) => temp(
                "Unexpected Kind",
                format!(
                    "this expression should be a {} but instead found a {}",
                    kind.to_string().success(),
                    kind.invert().to_string().error()
                ),
            ),
        }
    }
}
