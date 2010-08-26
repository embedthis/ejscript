/*
    jquery.ejs.js - Ejscript jQuery support
    http://www.ejscript.com/
  
    Copyright (c) 2010 Embedthis Software
    Dual licensed under GPL licenses and the Embedthis commercial license.
    See http://www.embedthis.com/ for further licensing details.
 */

;(function($) {

    /* Chainable functions */

    $.fn.extend({
        ejsSortTable: function(settings) {
            var defaults = {
                "data-sort-order": "ascending",
            };
            $.extend(defaults, settings || {});
            this.each(function() { 
                var e = $(this);
                var options = $.extend({}, defaults, e.data("ejs-options") || {}, getDataAttributes.apply(e));
                if (!options.sortConfig) {
                    if (options["data-sort"] != "false") {
                        var items = $("th", e).filter(':contains("' + options["data-sort"] + '")');
                        var el = $("th", e).filter(':contains("' + options["data-sort"] + '")').get(0);
                        if (el) {
                            var order = (options["data-sort-order"].indexOf("asc") >= 0) ? 0 : 1;
                            options.sortConfig = {sortList: [[el.cellIndex, order]]};
                        } else {
                            options.sortConfig = {sortList: [[0, 0]]};
                        }
                    }
                    e.tablesorter(options.sortConfig);
                }
                e.data("ejs-options", options);
            });
            return this;
        },

        ejsRefresh: function(settings) {
            function anim() {
                var e = $(this);
                options = e.data("ejs-options")
                var effects = options["data-effects"];
                if (effects == "highlight") {
                    e.fadeTo(0, 0.3);
                    e.fadeTo(750, 1);
                } else if (effects == "fadein") {
                    e.fadeTo(0, 0);
                    e.fadeTo(1000, 1);
                } else if (effects == "fadeout") {
                    e.fadeTo(0, 1);
                    e.fadeTo(1000, 0);
                } else if (effects == "bold") {
                    /* Broken */
                    e.css("font-weight", 100);
                    e.animate({"opacity": 0.1, "font-weight": 900}, 1000);
                }
            }

            function applyData(data, http) {
                var d, e, id, oldID, newElt, options, isHtml = false;

                var id = this.attr("id");
                var apply = this.attr('data-apply') || id;
                e = (apply) ? $('#' + apply) : $(this);

                var options = e.data("ejs-options");
                var contentType = http.getResponseHeader("Content-Type");

                if (contentType == "text/html") {
                    try {
                        /* Copy old element options and id */
                        data = $(data);
                        data.data("ejs-options", options);
                        data.attr("id", id);
                        copyDataAttributes.call(data, e);
                        isHtml = true;
                    } catch (e) {
                        console.debug(e);
                    }
                }
                if (isHtml) {
                    if (e[0] && e[0].tagName == "TABLE") {
                        /* Copy tablesorter options and config and re-sort the data before displaying */
                        data.data("tablesorter", e.data("tablesorter"));
                        var config = data[0].config = e[0].config;
                        data.tablesorter({ sortList: config.sortList });
                    }
                    e.replaceWith(data);
                    e = data;

                } else {
                    d = e[0];
                    if (d.type == "checkbox") {
                        if (data == e.val()) {
                            e.attr("checked", "yes")
                        } else {
                            e.removeAttr("checked")
                        }
                    } else if (d.type == "radio") {
                        //  MOB BROKEN
                        if (data == e.val()) {
                            e.attr("checked", "yes")
                        } else {
                            e.removeAttr("checked")
                        }
                    } else if (d.type == "text" || d.type == "textarea") {
                        e.val(data);
                    } else if (d.tagName == "IMG") {
                        e.attr("src", data)
                        // var img = $('<img />').attr('src', data);
                    } else {
                        e.text(data);
                    }
                }
                anim.call(e);
                return e;
            }

            function update(defaults) {
                var e = $(this);
                var doptions = e.data("ejs-options");
                var options = $.extend({}, defaults, e.data("ejs-options") || {}, getDataAttributes.apply(e));
                e.data("ejs-options", options);
                if (options.refresh) {
                    var method = options["data-refresh-method"] || "GET";
                    $.ajax({
                        url: options["data-refresh"],
                        type: method,
                        success: function (data, status, http) { 
                            if (http.status == 200) {
                                e = applyData.call(e, data, http); 
                            } else if (http.status == 0) {
                                log("Error updating control: can't connect to server");
                            } else {
                                log("Error updating control: " + http.statusText + " " + http.responseText); 
                            }
                        },
                        complete: function() {
                            setTimeout(function(e) { 
                                update.call(e, options); 
                            }, options["data-refresh-period"], e);
                        },
                        error: function (http, msg, e) { 
                            log("Error updating control: " + msg); 
                        },
                    });
                } else {
                    setTimeout(function(e) { update.call(e, options);}, options["data-refresh-period"], e);
                }
                if (!options.bound) {
                    $(document).bind('keyup.refresh', function(event) {
                        //  MOB -- should expose this as defaults
                        if (event.keyCode == options.toggle) {
                            $('[data-refresh]').ejsToggleRefresh();
                        }
                    });
                    options.bound = true;
                }
            }

            var defaults = {
                "period": 60000,
                "refresh": true,
                "toggle": 27,
            };
            $.extend(defaults, settings || {});
            this.each(function() { 
                update.call($(this), defaults);
            });
            return this;
        },

        ejsToggleRefresh: function() {
            this.each(function() {
                elt = $(this);
                var options = elt.data("ejs-options");
                if (options) {
                    options.refresh = !options.refresh;
                }
            /* FUTURE
                var image = $(".-ejs-table-download", e.get(0));
                if (options.refresh) {
                    image.src = image.src.replace(/red/, "green");
                } else {
                    image.src = image.src.replace(/green/, "red");
                }
            */
            });
        },
    });

    /************************************************* Support ***********************************************/
    /*
        Background request using data-* element attributes. Apply the result to the data-apply (or current element).
     */
    function backgroundRequest() {
        var el     = $(this),
            data   = el.is('form') ? el.serializeArray() : [],
            method = el.attr('method') || el.attr('data-method') || 'GET',
            /* Data-url is a modified remote URL */
            url    = el.attr('action') || el.attr('href') || el.attr('data-url') || el.attr('data-remote');

        if (url === undefined) {
            throw "No URL specified for remote call";
        }
        el.trigger('ajax:before');
        // MOB changeUrl(url);
        $.ajax({
            url: url,
            data: data,
            type: method.toUpperCase(),
            beforeSend: function (http) {
                el.trigger('ajax:loading', http);
            },
            success: function (data, status, http) {
                el.trigger('ajax:success', [data, status, http]);
                var apply = el.attr('data-apply');
                if (apply) {
                    $(apply).html(data)
                }
            },
            complete: function (http) {
                el.trigger('ajax:complete', http);
            },
            error: function (http, status, error) {
                el.trigger('ajax:failure', [http, status, error]);
            }
        });
        el.trigger('ajax:after');
    }

    /*
        Foreground request using data-* element attributes. This supports non-GET methods and use of 
        the SecurityToken
     */
    function request() {
        var el     = $(this);
        var method = el.attr('method') || el.attr('data-method') || 'GET';
        var url    = el.attr('action') || el.attr('href') || el.attr('data-click');
        var key    = el.attr('data-key');
        var addKey = el.attr('data-add-key');
        var params;

        if (url === undefined) {
            alert("No URL specified");
            return;
        }
        method = method.toUpperCase();
        if (key) {
            if (!addKey) {
                addKey = (method == "GET") ? "names" : null;
            }
        } else addKey = null;

        if (addKey == "names") {
            var keys = [];
            var split = key.split("&");
            for (i in split) {
                pair = split[i];
                keys.push(pair.split("=")[0]);
            } 
            if (keys.length > 0) {
                url = url + "/" + keys.join("/");
            }
        } else if (addKey == "pairs") {
            url = url + "?" + key;
        } else if (addKey == "params") {
            params = key.split("&");
        }
        if (method == "GET") {
            window.location = url;
        } else {
            var tokenName = $('meta[name=SecurityTokenName]').attr('content');
            var token = $('meta[name=' + tokenName + ']').attr('content');
            if (token) {
                var form = $('<form method="post" action="' + url + '"/>').
                    append('<input name="__method__" value="' + method + '" type="hidden" />').
                    append('<input name="' + tokenName + '" value="' + token + '" type="hidden" />');
                if (params) {
                    for (k in params) {
                        param = params[k];
                        pair = param.split("=")
                        form.append('<input name="' + pair[0] + '" value="' + pair[1] + '" type="hidden" />');
                    }
                }
                form.hide().appendTo('body').submit();
            } else {
                alert("Missing Security Token");
            }
        }
    }

    function copyDataAttributes(from) {
        this.each(function() {
            var e = $(this);
            $.each($(from)[0].attributes, function(index, att) {
                if (att.name.indexOf("data-") == 0) {
                    if (e.attr(att.name) == null) {
                        e.attr(att.name, att.value);
                    }
                }
            });
        });
    }

    function getDataAttributes() {
        var e = $(this);
        var result = {};
        $.each(e[0].attributes, function(index, att) {
            if (att.name.indexOf("data-") == 0) {
                result[att.name] = att.value;
            }
        });
        return result;
    }

    function log(msg) {
        // $('#console').append('<div>' + new Date() + ' ' + text + '</div>');
        if (window.console) {
            console.debug(msg);
        } else {
            alert(msg);
        }
    }

    /***************************************** Live Attach to Elements ***************************************/

    /* Click with data-confirm */
    $('a[data-confirm],input[data-confirm]').live('click', function () {
        var el = $(this);
        el.bind('confirm', function (evt) {
            return confirm(el.attr('data-confirm'));
        });
        el.trigger('confirm');
    });

    /* Click with tabs */
//  MOB -- should this be this or div#-ejs-tabs
    $('div.tabbed-dialog li').live('click', function(e) {
        var next = $(this).attr('data-remote');
        $('div[id|=pane]').hide();
        var pane = $('div#' + next);
        pane.fadeIn(500);
        pane.addClass('pane-visible');
        pane.removeClass('pane-hidden');
        e.preventDefault();
        return false
    });

    /* Click in form. Ensure submit buttons are added for remote calls */
    $('input[type=submit]').live('click', function (e) {
        $(this).attr("checked", true);
    });

    /* Click on table data-remote="" */
    $('table[data-remote] tr').live('click', function(e) {
        var table = $(this).parents("table");
        //  MOB -- remove this and use data-key
        var id = $(this).attr('data-id');
        if (!id) {
            id = $(this).parent().children().index($(this)) + 1;
        }
        table.attr('data-url', table.attr('data-remote') + "?id=" + id);
        backgroundRequest.apply(this);
        e.preventDefault();
    });

    /* Click with button data-remote */
    $('button[data-remote]').live('click', function(e) {
        backgroundRequest.apply(this);
        e.preventDefault();
    });

    /* Click on form with data-remote (background) */
    $('form[data-remote]').live('submit', function (e) {
        backgroundRequest.apply(this);
        e.preventDefault();
    });

    /* Click on link with data-remote (background) */
    $('a[data-remote],input[data-remote]').live('click', function (e) {
        backgroundRequest.apply(this);
        e.preventDefault();
    });

    /* Click on link foreground with data-method */
    $('a[data-method]:not([data-remote])').live('click', function (e) {
        request.apply(this)
/* UNUSED
        var link = $(this);
        var href = link.attr('href');
        var method = link.attr('data-method');
        var tokenName = $('meta[name=SecurityTokenName]').attr('content');
        var token = $('meta[name=' + tokenName + ']').attr('content');
        if (token) {
            $('<form method="post" action="' + url + '"/>').hide().
                append('<input name="__method__" value="' + method + '" type="hidden" />').
                append('<input name="' + tokenName + '" value="' + token + '" type="hidden" />').
                appendTo('body').
                submit();
        } else {
            alert("Missing Security Token");
        }
*/
        e.preventDefault();
    });

    /* Click on anything with data-click */
    $('[data-click]').live('click', function (e) {
        request.apply(this);
        e.preventDefault();
    });

/////////////////////////////////////////
/*  MOB -- TODO - fix location 
    //  MOB -- rename change Address
    function changeUrl(i) {
        console.log("CHANGE " + i);
        $.address.title(i);
    }
    $.address.strict(false);
    $.address.externalChange(function(e) {
        console.log("EXT CHANGE " + e.value);
        changeUrl(e.value);
    });
    $.address.change(function(e) {
        console.log("INT CHANGE " + e.value);
        $.address.title(e.value);
    });
    $.address.init(function(e) {
        console.log("INIT CALLBACK");
    });
    $.address.title("MOB");
    $.address.value("ABC");
*/

    /**************************** Initialization ************************/

    $(document).ready(function() {

        $('[data-sort-order]').ejsSortTable();
        $('[data-refresh]').ejsRefresh();

        $.each($('[data-modal]'), function() {
            this.modal({
                escClose: false, 
                overlayId: "-ejs-modal-background", 
                containerId: "-ejs-modal-foreground",
            });
        });
    });

})(jQuery);

