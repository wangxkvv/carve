/*
    Copyright 2008 Jeff Schiller

    This file is part of Carve.

    Carve is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Carve is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Carve.  If not, see http://www.gnu.org/licenses/ .

*/
#include "svghighlighter.h"

#include <QTextDocument>
#include <QRegExp>
#include <QStringList>

#include <iostream>
using std::cout;
using std::endl;

SVGHighlighter::SVGHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
{
    // from http://www.w3.org/TR/SVGTiny12/elementTable.html
    svgTags
        << "animateColor"
        << "animateMotion"
        << "animateTransform"
        << "animate"
        << "animation"
        << "audio"
        << "a"
        << "circle"
        << "defs"
        << "desc"
        << "discard"
        << "ellipse"
        << "font-face-src" // order here is important, longest should be checked first
        << "font-face-uri"
        << "font-face"
        << "font"
        << "foreignObject"
        << "g"
        << "glyph"
        << "handler"
        << "hkern"
        << "image"
        << "linearGradient"
        << "line"
        << "listener"
        << "metadata"
        << "missing-glyph"
        << "mpath"
        << "path"
        << "polygon"
        << "polyline"
        << "prefetch"
        << "radialGradient"
        << "rect"
        << "script"
        << "set"
        << "solidColor"
        << "stop"
        << "svg"
        << "switch"
        << "tbreak"
        << "text"
        << "textArea"
        << "title"
        << "tspan"
        << "use"
        << "video"
        ;

    // from http://www.w3.org/TR/SVGTiny12/attributeTable.html
    svgAttrs
            << "about"
            << "accent-height"
            << "accumulate"
            << "additive"
            << "alphabetic"
            << "arabic-form"
            << "ascent"
            << "attributeName"
            << "attributeType"
            << "audio-level"
            << "bandwidth"
            << "baseProfile"
            << "bbox"
            << "begin"
            << "buffered-rendering"
            << "by"
            << "calcMode"
            << "cap-height"
            << "class"
            << "color-rendering"
            << "content"
            << "contentScriptType"
            << "cx"
            << "cy"
            << "dataType"
            << "defaultAction"
            << "descent"
            << "direction"
            << "display-align"
            << "display"
            << "dur"
            << "editable"
            << "end"
            << "ev:event"
            << "event"
            << "externalResourcesRequired"
            << "fill-opacity"
            << "fill-rule"
            << "fill"
            << "filter" // SVG 1.2 Filters module
            << "focusHighlight"
            << "focusable"
            << "font-family"
            << "font-size"
            << "font-stretch"
            << "font-style"
            << "font-variant"
            << "font-weight"
            << "from"
            << "g1"
            << "g2"
            << "glyph-name"
            << "gradientUnits"
            << "handler"
            << "hanging"
            << "height"
            << "horiz-adv-x"
            << "horiz-origin-x"
            << "ideographic"
            << "id"
            << "image-rendering"
            << "initialVisibility"
            << "keyPoints"
            << "keySplines"
            << "keyTimes"
            << "lang"
            << "line-increment"
            << "mathematical"
            << "max"
            << "mediaCharacterEncoding"
            << "mediaContentEncoding"
            << "mediaSize"
            << "mediaTime"
            << "min"
            << "nav-down-left"
            << "nav-down-right"
            << "nav-down"
            << "nav-left"
            << "nav-next"
            << "nav-prev"
            << "nav-right"
            << "nav-up-left"
            << "nav-up-right"
            << "nav-up"
            << "observer"
            << "offset"
            << "origin"
            << "overlay"
            << "overline-position"
            << "overline-thickness"
            << "panose-1"
            << "pathLength"
            << "path"
            << "phase"
            << "playbackOrder"
            << "pointer-events"
            << "points"
            << "preserveAspectRatio"
            << "propagate"
            << "property"
            << "rel"
            << "repeatCount"
            << "repeatDur"
            << "requiredExtensions"
            << "requiredFeatures"
            << "requiredFonts"
            << "requiredFormats"
            << "resource"
            << "restart"
            << "rev"
            << "role"
            << "rotate"
            << "rx"
            << "ry"
            << "shape-rendering"
            << "slope"
            << "snapshotTime"
            << "solid-color"
            << "solid-opacity"
	    << "spreadMethod"
            << "stemh"
            << "stemv"
            << "stop-color"
            << "stop-opacity"
	    << "style"
            << "strikethrough-position"
            << "strikethrough-thickness"
            << "stroke-dasharray"
            << "stroke-dashoffset"
            << "stroke-linecap"
            << "stroke-linejoin"
            << "stroke-miterlimit"
            << "stroke-opacity"
            << "stroke-width"
            << "stroke"
            << "syncBehaviorDefault"
            << "syncBehavior"
            << "syncMaster"
            << "syncToleranceDefault"
            << "syncTolerance"
            << "systemLanguage"
            << "target"
            << "text-align"
            << "text-anchor"
            << "text-rendering"
            << "timelineBegin"
            << "to"
            << "transformBehavior"
            << "transform"
            << "typeof"
            << "type"
            << "u1"
            << "u2"
            << "underline-position"
            << "underline-thickness"
            << "unicode-bidi"
            << "unicode-range"
            << "unicode"
            << "units-per-em"
            << "values"
            << "vector-effect"
            << "version"
            << "viewBox"
            << "viewport-fill-opacity"
            << "viewport-fill"
            << "visibility"
            << "widths"
            << "width"
            << "x-height"
            << "x1"
            << "x2"
            << "x"
            << "xlink:actuate"
            << "xlink:arcrole"
            << "xlink:href"
            << "xlink:role"
            << "xlink:show"
            << "xlink:title"
            << "xlink:type"
            << "xmlns:xlink"
            << "xmlns:svg"
            << "xmlns:rdf"
            << "xmlns"
            << "xml:base"
            << "xml:id"
            << "xml:lang"
            << "xml:space"
            << "y1"
            << "y2"
            << "y"
            << "zoomAndPan"

            << "color"
            << "opacity"
            << "d"
            << "r"
            << "k"
            ;

    // nicked from Mozilla's view-source
    this->svgElemFormat.setForeground(Qt::darkMagenta);
    this->svgElemFormat.setFontWeight(QFont::Bold);

    this->svgAttrFormat.setForeground(Qt::black);
    this->svgAttrFormat.setFontWeight(QFont::Bold);

    this->stringFormat.setForeground(Qt::blue);

    this->commentFormat.setForeground(Qt::darkGreen);
    this->commentFormat.setFontItalic(true);

    this->malformedFormat.setForeground(Qt::red);
    this->malformedFormat.setFontItalic(true);

    this->textContentsFormat.setForeground(Qt::black);
}


/*

  x+ = 1 or more
  x* = 0 or more

  xmldoc = preamble doctype entity_decl node

  node = closed_node

  self_closing_node = '<' tagname (ws)+ (attr)* '/>'
  closed_node = start_tag node_contents* end_tag

  start_tag = '<' tagname (ws)+ (attr)* '>'
  end_tag = '</' tagname '>'
  node_contents = (ws)* (comment | node | text_contents) (ws)* (node_contents)*

  State Machine:
  START:
    - search for comment: "<!--", highlight as comment, go to IN_COMMENT_PREDOC
    - search for first opening tag:  "<svg[ws|/>|>]", highlight "svg", set state to IN_START_TAG, push 'svg' onto the tag stack
    - if a non-SVG tag is found, then set state to MALFORMED

  IN_COMMENT_PREDOC:
    - search for end comment:  "-->", highlight as comment, go to START

  IN_START_TAG:
    - search for each attr: "[ws]*[a-zA-z][a-zA-Z0-9]*=", highlight from first character to equals sign, set to IN_ATTR_VALUE
    - if no attrs found, then search for "/>", pop off tag stack:
        - if it was empty prior to pop, go to MALFORMED
        - if it is not empty, go to IN_NODE
        - if it is now empty, go to POSTDOC
    - if no attrs found and no "/>", then search for ">", go to IN_NODE if found

  IN_ATTR_VALUE:
    - search for string value:  "\"[alphanum]*\"", highlight entire text (determine if SVG attr or not), set to IN_START_TAG
    - if not found, search for string value "\'[alphanum]*\'", highlight entire text, set to IN_START_TAG
    - if "<", ">" found, then go to MALFORMED

  IN_NODE:
    - search for closing tag: "</[tag stack top]>", highlight text up to closing tag as text contents, highlight from third character
      to second-last character as a tag (determine if SVG tag or not), pop from tag stack:
        - if it was empty prior to pop, go to MALFORMED
        - if it is not empty, go to IN_NODE
        - if it is now empty, go to POSTDOC
    - search for opening tag: "<[a-zA-Z][a-zA-Z]*[ws|/>|>]", highlight up to "<" as text contents, highlight from the 2nd character to
      end of alphanum (determine if SVG tag or not), push onto tag stack, go to IN_START_TAG
    - search for comment: "<!--", highlight as comment, go to IN_COMMENT
    - else, highlight as text contents

  IN_COMMENT:
    - search for closing comment: "-->", highlight as comment, go to IN_NODE
    - highlight as comment

  POSTDOC:
    - search for comment: "<!--", highlight as comment, go to IN_COMMENT_POSTDOC

  IN_COMMENTPOSTDOC:
    - search for end comment:  "-->", highlight as comment, go to POSTDOC

  MALFORMED: highlight all text red and italic, never leave this state

*/

enum SVGHighlighterState {
    START = -1,
    IN_COMMENT_PREDOC = 0,
    IN_START_TAG = 1,
    IN_ATTR_VALUE_SQ = 2,
    IN_ATTR_VALUE_DQ = 3,
    IN_NODE = 4,
    IN_COMMENT = 5,
    POSTDOC = 6,
    IN_COMMENTPOSTDOC = 7,
    MALFORMED = 100,
};

QRegExp rStartComment("\\<\\!\\-\\-");
QRegExp rEndComment("\\-\\-\\>");
QRegExp rStartDoc("<svg(\\s|\\f|\\n|\\r|/>|>|$)");
QRegExp rSelfClosing("/>");
QRegExp rCloseStartTag(">");
QRegExp rStartTag("<[a-zA-Z]([a-zA-Z0-9]|\\:)*(\\s|\\f|\\n|\\r|/>|>|$)");
QRegExp rCloseTag("</[a-zA-Z]([a-zA-Z0-9]|\\:)*>");
QRegExp rAttrName("[a-zA-Z](\\w|\\:|\\-)*=");

void SVGHighlighter::highlightBlock(const QString &text) {
    int index = 0;
    int state = this->previousBlockState();

//    cout << "=======" << endl;
    while(index < text.size()) {
        switch(state) {
            case START: {
                int svgPos = rStartDoc.indexIn(text, index);
                int commentIndex = rStartComment.indexIn(text, index);
		if( commentIndex >= index && (svgPos < index || commentIndex < svgPos)) {
                    this->setFormat(commentIndex, 4, this->commentFormat);
                    state = IN_COMMENT_PREDOC;
                    index = commentIndex+4;
                }
                else if( svgPos >= index && (commentIndex < index || svgPos < commentIndex)) {
                    this->setFormat(svgPos+1, 3, this->svgElemFormat);
                    state = IN_START_TAG;
                    index = svgPos+4;
                }
                else {
                    state = START;
                    index = text.size();
                }
                break; }
            case IN_COMMENT_PREDOC: {
                int endComment = rEndComment.indexIn(text, index);
                if(endComment >= index) {
                    this->setFormat(index, endComment+3, this->commentFormat);
                    index = endComment+3;
                    state = START;
                }
                else {
                    this->setFormat(index, text.size(), this->commentFormat);
                    state = IN_COMMENT_PREDOC;
                    index = text.size();
                }
                break; }
            case IN_START_TAG: {
                int attrIndex = rAttrName.indexIn(text, index);
                int selfClosingIndex = rSelfClosing.indexIn(text, index);
                int closeOpenTagIndex = rCloseStartTag.indexIn(text, index);
		// found an attribute
		if( attrIndex >= index &&
                        ( (selfClosingIndex < index && closeOpenTagIndex < index) ||
                          (selfClosingIndex < index && attrIndex < closeOpenTagIndex) ||
                          (attrIndex < selfClosingIndex && closeOpenTagIndex < index) ||
                          (attrIndex < selfClosingIndex && attrIndex < closeOpenTagIndex) ) )
                {
                    QRegExp eq_sq("='"), eq_dq("=\"");
                    int equalIndex_sq = eq_sq.indexIn(text, attrIndex);
                    int equalIndex_dq = eq_dq.indexIn(text, attrIndex);
                    if(equalIndex_sq < index && equalIndex_dq < index) {
                        state = MALFORMED;
                    }
                    else {
                        int equalIndex = -1;
                        if(equalIndex_sq >= index && (equalIndex_dq < index || equalIndex_dq > equalIndex_sq)) {
                            equalIndex = equalIndex_sq;
                            state = IN_ATTR_VALUE_SQ;
                        }
                        else {
                            equalIndex = equalIndex_dq;
                            state = IN_ATTR_VALUE_DQ;
                        }
                        QString attrName = text.right(text.size()-attrIndex).left(equalIndex-attrIndex);
                        index = equalIndex+1;
                        bool bFoundAttr = false;
                        for (int i = 0; i < this->svgAttrs.size(); ++i) {
                            if(attrName == this->svgAttrs.at(i)) {
                                bFoundAttr = true;
                                break;
                            }
                        }
                        if(bFoundAttr) {
                            this->setFormat(attrIndex, (equalIndex-attrIndex), this->svgAttrFormat);
                        }
                    }
                }
		// found self-closing tag />
                else if( selfClosingIndex >= index &&
                          ( (attrIndex < index && closeOpenTagIndex < index) ||
                            (selfClosingIndex < attrIndex && closeOpenTagIndex < index) ||
                            (attrIndex < index && selfClosingIndex < closeOpenTagIndex) ||
			    (selfClosingIndex < attrIndex && selfClosingIndex < closeOpenTagIndex) ) )
                {
                    index = selfClosingIndex+2;
                    state = IN_NODE;
                }
		// end of open tag (now inside node)
                else if( closeOpenTagIndex >= index &&
                         ( (attrIndex < index && selfClosingIndex < index) ||
                           (closeOpenTagIndex < attrIndex && selfClosingIndex < index) ||
                           (attrIndex < index && closeOpenTagIndex < selfClosingIndex) ||
                           (closeOpenTagIndex < attrIndex && closeOpenTagIndex < selfClosingIndex) ) )
                {
                    index = closeOpenTagIndex+1;
                    state = IN_NODE;
                }
                else {
                    index = text.size();
                }
                break; }
            case IN_ATTR_VALUE_SQ: {
                QRegExp rEndS("\\\'");
                int endIndex = rEndS.indexIn(text, index+1);
		if(endIndex >= index) {
                    this->setFormat(index, endIndex-index, this->stringFormat);
                    index = endIndex + 1;
                    state = IN_START_TAG;
                }
                else {
                    this->setFormat(index, text.size()-index, this->stringFormat);
                    index = text.size();
                }
                break; }
            case IN_ATTR_VALUE_DQ: {
                QRegExp rEndS("\\\"");
                int endIndex = rEndS.indexIn(text, index+1);
		if(endIndex >= index) {
                    this->setFormat(index, endIndex-index, this->stringFormat);
                    index = endIndex + 1;
                    state = IN_START_TAG;
                    }
                else {
                    this->setFormat(index, text.size()-index, this->stringFormat);
                    index = text.size();
                }
                break; }
            case IN_NODE: {
                int commentIndex = rStartComment.indexIn(text, index);
                int closeTagIndex = rCloseTag.indexIn(text, index);
                int startTagIndex = rStartTag.indexIn(text, index);
                if( commentIndex >= index &&
                        ( (closeTagIndex < index && startTagIndex < index) ||
                          (commentIndex < closeTagIndex && startTagIndex < index) ||
                          (closeTagIndex < index && commentIndex < startTagIndex) ||
                          (commentIndex < closeTagIndex && commentIndex < startTagIndex) ) )
                {
                    this->setFormat(commentIndex, 4, this->commentFormat);
                    state = IN_COMMENT;
                    index = commentIndex+4;
                }
                else if( closeTagIndex >= index &&
                        ( (commentIndex < index && startTagIndex < index) ||
                          (closeTagIndex < commentIndex && startTagIndex < index) ||
                          (commentIndex < index && closeTagIndex < startTagIndex) ||
                          (closeTagIndex < commentIndex && closeTagIndex < startTagIndex) ) )
                {
                    QRegExp rEndCloseTag(">");
                    int endCloseTagIndex = rEndCloseTag.indexIn(text, closeTagIndex+2);
                    this->setFormat(index, closeTagIndex-index, this->textContentsFormat);
                    QString tagName( text.right(text.size()-closeTagIndex-2).left(endCloseTagIndex-closeTagIndex-2) );

                    bool bFoundElem = false;
                    for (int i = 0; i < this->svgTags.size(); ++i) {
                        if(tagName == this->svgTags.at(i)) {
                            bFoundElem = true;
                            break;
                        }
                    }
                    if(bFoundElem) {
                        this->setFormat(closeTagIndex+2, endCloseTagIndex-closeTagIndex-2, this->svgElemFormat);
                    }
                    index = endCloseTagIndex+1;
                    state = IN_NODE;
                }
                else if( startTagIndex >= index &&
                         ( (commentIndex < index && closeTagIndex < index) ||
                           (startTagIndex < commentIndex && closeTagIndex < index) ||
                           (commentIndex < index && startTagIndex < closeTagIndex) ||
                           (startTagIndex < commentIndex && startTagIndex < closeTagIndex) ) )
                {
                    this->setFormat(index, startTagIndex - index, this->textContentsFormat);
                    QRegExp sTagNameDone("(\\s|\\f|\\n|\\r|/>|>)");
                    startTagIndex += 1;
                    index = sTagNameDone.indexIn(text, startTagIndex);
                    QString tagName( text.right(text.size()-startTagIndex).left(index-startTagIndex) );
                    bool bFoundElem = false;
                    for (int i = 0; i < this->svgTags.size(); ++i) {
                        if(tagName == this->svgTags.at(i)) {
                            bFoundElem = true;
                            break;
                        }
                    }
                    if(bFoundElem) {
                        this->setFormat(startTagIndex, tagName.size(), this->svgElemFormat);
                    }
                    state = IN_START_TAG;
                }
                else {
                    index = text.size();
                }
                break; }
            case IN_COMMENT: {
                int endComment = rEndComment.indexIn(text, index);
                if(endComment >= index) {
                    this->setFormat(index, endComment+3, this->commentFormat);
                    index = endComment+3;
                    state = IN_NODE;
                }
                else {
                    this->setFormat(index, text.size(), this->commentFormat);
                    state = IN_COMMENT;
                    index = text.size();
                }
                break; }
            case POSTDOC: {
                cout << "Somehow got to POSTDOC" << endl;
                index = text.size();
                break; }
            case MALFORMED: {
                this->setFormat(index, (text.size()-index), this->malformedFormat);
                index = text.size();
                break; }
            default: {
                index = text.size();
                break; }
        } // switch
    } // while
    this->setCurrentBlockState(state);
}
