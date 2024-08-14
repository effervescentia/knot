use super::{code::ToCode, CodeFrame, Display, ErrorCode, ErrorDisplay};
use kore::{color::Highlight, format::Indented, str};
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
            Self::UntypedParameter => ErrorCode::UNTYPED_PARAMETER,
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
        let bind = |title, description, suggestion| {
            let module = modules.get(&id.0);
            let range = nodes.get(id);

            let code_frame = match (module, range) {
                (Some((link, text)), Some(range)) => {
                    Some(CodeFrame::color(root_dir, link, text, *range))
                }
                _ => None,
            };

            ErrorDisplay {
                code: self.to_code(),
                title,
                description,
                suggestion,
                code_frame,
            }
        };

        match self {
            Self::NotInferrable(_) => bind(
                "Not Inferrable",
                str!("the type of this expression could not be inferred from other types"),
                None,
            ),

            Self::NotFound(name) => bind(
                "Not Found",
                format!(
                    "This expression references a variable named {} which does not exist.",
                    name.error()
                ),
                Some(format!(
                    "You may have misspelled the variable name or forgotten to declare it.

The {} keyword can be used to declare a local variable.

{}",
                    "let".success(),
                    Indented(format!("let {} = 123;", name).success())
                )),
            ),

            // TODO: use a two-target code frame
            Self::VariantNotFound(_, variant) => bind(
                "Variant Not Found",
                format!(
                    "enumerator X does not have a variant named {}",
                    variant.error()
                ),
                None,
            ),

            // TODO: use a two-target code frame
            Self::DeclarationNotFound(_, name) => bind(
                "Declaration Not Found",
                format!(
                    "This module does not contain a declaration named {}.",
                    name.error()
                ),
                None,
            ),

            // TODO: use a two-target code frame
            Self::NotIndexable(_, name) => bind(
                "Not Indexable",
                // TODO: fix this error message
                format!(
                    "The property {} cannot be accessed because this type of value does not named",
                    name.error()
                ), // format!("the property {} cannot be accessed because this type of value does not support named properties", name.error())
                None,
            ),

            // TODO: use a two-target code frame
            Self::PropertyNotFound(_, name) => bind(
                "Property Not Found",
                format!("a property with the name {} was not found", name.error()),
                None,
            ),

            Self::DuplicateProperty(name) => bind(
                "Duplicate Property",
                format!(
                    "a property with the name {} has already been declared",
                    name.error()
                ),
                None,
            ),

            Self::NotSpreadable(_) => bind(
                "Not Spreadable",
                str!("only object-like types can be spread"),
                None,
            ),

            Self::UntypedParameter => bind(
                "Untyped Parameter",
                str!("all function parameters must have an explicit type"),
                None,
            ),

            Self::DefaultValueRejected(_) => bind(
                "Default Value Rejected",
                "the type of the default value does not match the declared type of the parameter"
                    .to_string(),
                None,
            ),

            Self::NotCallable(_) => bind(
                "Not Callable",
                str!("this expression is not a function and cannot be called"),
                None,
            ),

            Self::UnexpectedArgument(_) => bind(
                "Unexpected Argument",
                str!("this function call expects fewer arguments than were provided"),
                None,
            ),

            Self::MissingArgument(_) => bind(
                "Missing Argument",
                str!("this function call expects an additional argument that was not provided"),
                None,
            ),

            Self::ArgumentRejected(_, _) => bind(
                "Argument Rejected",
                str!(
                    "this argument did not match the expected type based on the function signature"
                ),
                None,
            ),

            Self::NotRenderable(_) => bind(
                "Not Renderable",
                str!("this expression is not a component and cannot be rendered"),
                None,
            ),

            Self::InvalidComponent(name) => bind(
                "Invalid Component",
                format!(
                    "the variable {} does not reference a valid component type",
                    name.error()
                ),
                None,
            ),

            Self::ComponentTypo(start_tag, end_tag) => bind(
                "Component Typo",
                format!(
                    "the start ({}) and end ({}) tags of this component do not match",
                    start_tag.error(),
                    end_tag.error(),
                ),
                None,
            ),

            Self::InvalidAttributes(_) => bind("Invalid Attributes", format!(""), None),

            Self::UnexpectedAttribute(name) => bind(
                "Unexpected Attribute",
                format!(
                    "this component does not accept an attribute named {}",
                    name.error()
                ),
                None,
            ),

            Self::MissingAttribute(_) => bind(
                "Missing Attribute",
                str!("this component expected an attribute that was not provided"),
                None,
            ),

            Self::AttributeRejected(_, _) => bind(
                "Attribute Rejected",
                "this attribute did not match the expected type based on the component signature"
                    .to_owned(),
                None,
            ),

            Self::BinaryOperationNotSupported(op, _, _) => bind(
                "Binary Operation Not Supported",
                format!(
                    "the operator {} cannot be applied to the arguments provided",
                    op.to_string().highlight()
                ),
                None,
            ),

            Self::UnaryOperationNotSupported(op, _) => bind(
                "Unary Operation Not Supported",
                format!(
                    "the operator {} cannot be applied to the argument provided",
                    op.to_string().highlight()
                ),
                None,
            ),

            Self::UnexpectedKind(_, kind) => bind(
                "Unexpected Kind",
                format!(
                    "this expression should be a {} but instead found a {}",
                    kind.to_string().success(),
                    kind.invert().to_string().error()
                ),
                None,
            ),
        }
    }
}
