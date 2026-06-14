// ============================================================
//  Block Clock – Case v1
//  Heltec Vision Master E290  (87 × 37 × 12.5 mm board)
//
//  Adapted from Freedom Clock Case v5.
//  Changes:
//    - Back text: "BLOCK CLOCK"
//    - Front logo: square-frame mark (outer square / inner hollow)
//
//  Board orientation:
//    Display face            → case FRONT  (Z = 0)
//    Board left / USB-C      → case LEFT   (X = 0 short end)
//    Buttons (RST/21/BOOT)   → BACK COVER – living-hinge islands
//
//  Physical orientation (as held / as printed):
//    USER'S TOP    = model Y = 0      = wall_b side  (USB-C entrance)
//    USER'S BOTTOM = model Y = case_w = button side  (living-hinge beams)
//
//  Back cover: snap-fit plug + 3 integrated living-hinge buttons.
//  Corner bosses: 4 × (6×4×1 mm) pads at inner corners, 2 mm mount hole.
// ============================================================

$fn = 64;

// ── Board ──────────────────────────────────────────────────
board_l = 87.0;
board_w = 37.0;
board_h = 12.5;

// ── Display ────────────────────────────────────────────────
disp_w   = 68.0;
disp_h   = 30.0;
disp_x   = 7.0;
disp_y   = (board_w - disp_h) / 2;
disp_from_right = board_l - disp_x - disp_w;

disp_win_ml  = 1.0;
disp_win_mr  = 1.25;
disp_win_mt  = 1.50;
disp_win_mb  = 1.0;

// ── Shell ──────────────────────────────────────────────────
wall       = 2.0;
wall_b     = 2.25;
wall_bot   = 2.25;
front_wall = 1.5;
tol   = 0.25;
R     = 3.5;
rear  = 1.4;

inner_l = board_l + 2*tol;
inner_w = board_w + 2*tol;
case_l  = inner_l + 2*wall;
case_w  = inner_w + 2*wall;
case_d  = front_wall + board_h + rear;

front_bevel = 2.0;

// ── USB-C (LEFT short end, X = 0 face) ─────────────────────
usb_cw  = 8.7;
usb_ch  = 2.5;
usb_cr  = 1.5;
usb_z0  = wall + 2.75;
usb_bev = 1.5;

// ── Cover panel ─────────────────────────────────────────────
cover_wall = 2.0;

// ── Post cross-section shape ─────────────────────────────────
POST_SHAPE = "d";

// ── Text on cover back face ──────────────────────────────────
text_str   = "BLOCK CLOCK";
text_size  = 6.0;
text_depth = 0.8;

// ── Buttons ──────────────────────────────────────────────────
btn_x        = [13.0, 20.0, 27.0];
btn_y        = case_w - 4.6;
btn_post_d   = 3.5;
btn_post_h   = 7.75;
btn_cap_d    = 0.0;
btn_cap_h    = 1.0;
btn_icon_r   = 1.8;
btn_icon_lw  = 0.55;
btn_icon_d   = 0.5;
btn_cut_w    = 1.0;
btn_beam_hw  = 3.0;
btn_beam_h   = 8.0;
btn_strip_h  = 1.5;
btn_pocket_d = 0.5;

// ── Corner mounting bosses ────────────────────────────────
corner_l = 6.0;
corner_w = 4.0;
corner_h = 1.0;
corner_d = 2.0;

// ── Corner board supports ─────────────────────────────────
supp_l = 4.5;
supp_h = 10.5;

// ── Snap plug ──────────────────────────────────────────────
plug_tol  = 0.15;
plug_d    = 3.5;
plug_wall = 1.2;
tab_w     = 9.0;
tab_nib      = 1.4;
tab_slot     = 0.6;
tab_x_bot    = [0.50];
tab_x_top    = [0.25, 0.75];
tab_y_fracs  = [0.25];
ctab_x_bot   = [0.25, 0.75];
ctab_x_top   = [0.50];
ctab_y_fracs = [0.75];
indent_d  = 1.2;
indent_h  = 2.0;

// ── Logo ───────────────────────────────────────────────────
logo_inset  = 0.8;
logo_cx     = (wall + disp_x + tol + disp_w - disp_win_mr + case_l) / 2 - 0.5;
logo_cy     = case_w / 2;

// ─────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────

module rbox(l, w, d, rv = R) {
    hull()
        for (x = [rv, l-rv], y = [rv, w-rv])
            translate([x, y, 0]) cylinder(r=rv, h=d);
}

module outer_shell() {
    hull() {
        translate([front_bevel, front_bevel, 0])
            rbox(case_l-2*front_bevel, case_w-2*front_bevel, 0.01, R-front_bevel);
        translate([0, 0, front_bevel])
            rbox(case_l, case_w, case_d-front_bevel, R);
    }
}

module usb_oval(t = wall+0.2) {
    cr    = min(usb_cr, usb_ch/2, usb_cw/2);
    cr_o  = cr + usb_bev;
    hull() {
        for (y = [cr, usb_cw - cr])
            for (z = [cr, usb_ch - cr])
                translate([t - 0.01, y, z])
                    rotate([0, 90, 0]) cylinder(r = cr, h = 0.01);
        for (y = [cr_o, usb_cw + 2*usb_bev - cr_o])
            for (z = [cr_o, usb_ch + 2*usb_bev - cr_o])
                translate([0, y - usb_bev, z - usb_bev])
                    rotate([0, 90, 0]) cylinder(r = cr_o, h = 0.01);
    }
}

// ── Block Clock logo: square frame (outer square with hollow center) ──
// Matches the drawBlockClockMark() icon used on the e-ink display.
module block_clock_logo(h = logo_inset) {
    ro   = 4.5;    // half outer side (9 mm × 9 mm outer square)
    wall_t = 1.3;  // frame wall thickness
    linear_extrude(h) {
        difference() {
            square([ro*2, ro*2], center = true);
            square([ro*2 - 2*wall_t, ro*2 - 2*wall_t], center = true);
        }
    }
}

// ─────────────────────────────────────────────────────────
//  Snap-tab indents inside main body long walls
// ─────────────────────────────────────────────────────────

module snap_indents() {
    iz = case_d - cover_wall - plug_d + 1.75;
    for (frac = tab_x_bot) {
        xc = wall + inner_l * frac;
        translate([xc - tab_w/2, wall_b - indent_d, iz])
            cube([tab_w, indent_d + 0.05, indent_h]);
    }
    for (frac = tab_x_top) {
        xc = wall + inner_l * frac;
        translate([xc - tab_w/2, case_w - wall_bot, iz])
            cube([tab_w, indent_d + 0.05, indent_h]);
    }
    for (frac = tab_y_fracs) {
        yc = wall_b + (case_w - wall_b - wall) * frac;
        translate([wall - indent_d,   yc - tab_w/2, iz]) cube([indent_d + 0.05, tab_w, indent_h]);
        translate([wall + inner_l,    yc - tab_w/2, iz]) cube([indent_d + 0.05, tab_w, indent_h]);
    }
}

// ─────────────────────────────────────────────────────────
//  Corner mounting bosses
// ─────────────────────────────────────────────────────────

module corner_bosses() {
    for (xi = [0, 1], yi = [0, 1]) {
        x0 = xi == 0 ? wall - tol : wall + inner_l + tol - corner_w;
        y0 = yi == 0 ? wall_b - tol : case_w - wall_bot + tol - corner_l;
        translate([x0, y0, front_wall])
            cube([corner_w, corner_l, corner_h]);
    }
}

// ─────────────────────────────────────────────────────────
//  Display window cut
// ─────────────────────────────────────────────────────────

module display_window_cut() {
    ox = wall + disp_x - tol + disp_win_ml;
    oy = wall_b + disp_y - tol + disp_win_mb;
    dw = disp_w + 2*tol - disp_win_ml - disp_win_mr;
    dh = disp_h + 2*tol - disp_win_mt - disp_win_mb;
    bev = front_wall;
    translate([ox, oy, -0.05])
        hull() {
            translate([-bev, -bev, 0]) cube([dw+2*bev, dh+2*bev, 0.01]);
            cube([dw, dh, front_wall + 0.1]);
        }
}

// ─────────────────────────────────────────────────────────
//  Main Body
// ─────────────────────────────────────────────────────────

module main_body() {
    difference() {
        outer_shell();
        translate([wall-tol, wall_b-tol, front_wall])
            cube([inner_l+2*tol, case_w-wall_b-wall_bot+2*tol, case_d]);
        display_window_cut();
        translate([-0.1, wall_b + (board_w - usb_cw)/2 + 0.00, usb_z0])
            usb_oval();
        snap_indents();
        translate([logo_cx, logo_cy, -0.05])
            block_clock_logo(h = logo_inset + 0.05);
    }
    corner_bosses();
}

// ─────────────────────────────────────────────────────────
//  Back Cover with living-hinge buttons
// ─────────────────────────────────────────────────────────

plug_l  = inner_l - 2*plug_tol;
plug_w  = case_w - wall_b - wall_bot - 2*plug_tol;
plug_ox = (case_l - plug_l) / 2;
plug_oy = wall_b + plug_tol;

module button_icons() {
    r  = btn_icon_r;
    lw = btn_icon_lw;
    d  = btn_icon_d;
    // Left – square frame
    translate([btn_x[0], btn_y, -0.05])
        linear_extrude(d + 0.05)
            difference() {
                square([r*2, r*2], center = true);
                square([r*2 - 2*lw, r*2 - 2*lw], center = true);
            }
    // Middle – X
    translate([btn_x[1], btn_y, -0.05])
        linear_extrude(d + 0.05)
            union() {
                rotate([0, 0,  45]) square([r*2, lw], center = true);
                rotate([0, 0, -45]) square([r*2, lw], center = true);
            }
    // Right – ring
    translate([btn_x[2], btn_y, -0.05])
        linear_extrude(d + 0.05)
            difference() {
                circle(r = r, $fn = 48);
                circle(r = r - lw, $fn = 48);
            }
}

module button_straight_cuts() {
    bx0 = btn_x[0] - btn_beam_hw - btn_cut_w;
    bx1 = btn_x[2] + btn_beam_hw + btn_cut_w;
    by_hcut = case_w - btn_strip_h - btn_beam_h - btn_cut_w;
    by_vbot = by_hcut + btn_cut_w;
    by_vtop = case_w - btn_strip_h;

    translate([bx0, by_hcut, -0.05])
        cube([bx1 - bx0, btn_cut_w, cover_wall + 0.1]);

    for (bx = [bx0,
               btn_x[0] + btn_beam_hw,
               btn_x[1] + btn_beam_hw,
               btn_x[2] + btn_beam_hw])
        translate([bx, by_vbot, -0.05])
            cube([btn_cut_w, by_vtop - by_vbot, cover_wall + 0.1]);

    translate([bx0, by_hcut, cover_wall - btn_pocket_d])
        cube([bx1 - bx0, by_vtop - by_hcut + 0.05, btn_pocket_d + 0.05]);
}

module _corner_supports() {
    for (xi = [0, 1]) {
        xc = xi == 0 ? plug_ox : plug_ox + plug_l;
        sx = xi == 0 ?  supp_l : -supp_l;
        translate([xc, plug_oy, cover_wall])
            linear_extrude(supp_h)
                polygon([[0, 0], [sx, 0], [0,  supp_l]]);
        translate([xc, plug_oy + plug_w, cover_wall])
            linear_extrude(supp_h)
                polygon([[0, 0], [sx, 0], [0, -supp_l]]);
    }
}

module _btn_wall_height_cuts() {
    bx0 = btn_x[0] - btn_beam_hw - btn_cut_w - plug_ox;
    bx1 = btn_x[2] + btn_beam_hw + btn_cut_w - plug_ox;
    bw  = bx1 - bx0;
    yw  = plug_w - plug_wall - 0.01;
    pw  = plug_wall + 0.02;
    translate([bx0, yw, -0.01])
        cube([bw, pw, 1.01]);
}

module _btn_post_body(h) {
    r = btn_post_d / 2;
    if (POST_SHAPE == "d") {
        intersection() {
            cylinder(r = r, h = h, $fn = 32);
            translate([-r, -r, 0]) cube([r*2, r, h]);
        }
    } else if (POST_SHAPE == "quadrant") {
        intersection() {
            cylinder(r = r, h = h, $fn = 32);
            translate([-r, -r, 0]) cube([r, r, h]);
        }
    } else {
        cylinder(r = r, h = h, $fn = 32);
    }
}

module back_cover() {
    difference() {
        rbox(case_l, case_w, cover_wall);
        button_straight_cuts();
        button_icons();
        translate([case_l/2, case_w/2, -0.05])
            linear_extrude(text_depth + 0.05)
                mirror([1, 0, 0])
                    text(text_str, size = text_size,
                         font = "Liberation Sans:style=Bold",
                         halign = "center", valign = "center");
    }

    for (bx = btn_x)
        translate([bx, btn_y, cover_wall - btn_pocket_d - 0.1]) {
            _btn_post_body(btn_post_h + 0.1);
            translate([0, 0, btn_post_h + 0.1])
                cylinder(d = btn_post_d, h = btn_cap_h, $fn = 32);
        }

    translate([plug_ox, plug_oy, cover_wall]) {
        difference() {
            cube([plug_l, plug_w, plug_d]);
            translate([plug_wall, plug_wall, -0.1])
                cube([plug_l-2*plug_wall, plug_w-2*plug_wall, plug_d+0.2]);
            _plug_slots();
            _plug_slots_y();
            _btn_wall_height_cuts();
        }
    }

    for (frac = ctab_x_bot) {
        xc = plug_ox + plug_l * frac;
        _snap_tab(xc, plug_oy, false);
    }
    for (frac = ctab_x_top) {
        xc = plug_ox + plug_l * frac;
        _snap_tab(xc, plug_oy + plug_w - plug_wall, true);
    }

    for (frac = ctab_y_fracs) {
        yc = plug_oy + plug_w * frac;
        _snap_tab_y(yc, plug_ox, false);
        _snap_tab_y(yc, plug_ox + plug_l - plug_wall, true);
    }

    _corner_supports();
}

module _plug_slots() {
    for (frac = ctab_x_bot) {
        xc = plug_l * frac;
        for (dx = [-(tab_w/2+tab_slot), tab_w/2])
            translate([xc+dx, -0.1, -0.1]) cube([tab_slot, plug_wall+0.2, plug_d+0.2]);
    }
    for (frac = ctab_x_top) {
        xc = plug_l * frac;
        for (dx = [-(tab_w/2+tab_slot), tab_w/2])
            translate([xc+dx, plug_w-plug_wall-0.1, -0.1]) cube([tab_slot, plug_wall+0.2, plug_d+0.2]);
    }
}

module _plug_slots_y() {
    for (frac = ctab_y_fracs) {
        yc = plug_w * frac;
        for (dy = [-(tab_w/2+tab_slot), tab_w/2]) {
            translate([-0.1,               yc+dy, -0.1]) cube([plug_wall+0.2, tab_slot, plug_d+0.2]);
            translate([plug_l-plug_wall-0.1, yc+dy, -0.1]) cube([plug_wall+0.2, tab_slot, plug_d+0.2]);
        }
    }
}

module _snap_tab(xc, y0, flip) {
    translate([xc - tab_w/2, y0, cover_wall])
    union() {
        cube([tab_w, plug_wall, plug_d]);
        hull() {
            translate([0, flip ? plug_wall + tab_nib : -tab_nib, plug_d - 1.5])
                cube([tab_w, 0.01, 0.01]);
            translate([0, flip ? plug_wall : 0, plug_d - 1.5])
                cube([tab_w, 0.01, 0.01]);
            translate([0, flip ? plug_wall : 0, plug_d])
                cube([tab_w, 0.01, 0.01]);
        }
    }
}

module _snap_tab_y(yc, x0, is_right) {
    translate([x0, yc - tab_w/2, cover_wall])
    union() {
        cube([plug_wall, tab_w, plug_d]);
        hull() {
            translate([is_right ? plug_wall + tab_nib : -tab_nib, 0, plug_d - 1.5])
                cube([0.01, tab_w, 0.01]);
            translate([is_right ? plug_wall : 0, 0, plug_d - 1.5])
                cube([0.01, tab_w, 0.01]);
            translate([is_right ? plug_wall : 0, 0, plug_d])
                cube([0.01, tab_w, 0.01]);
        }
    }
}

// ─────────────────────────────────────────────────────────
//  Render
// ─────────────────────────────────────────────────────────

PART = "both";   // -D 'PART="body"' | 'PART="cover"'

if      (PART == "body")  main_body();
else if (PART == "cover") back_cover();
else {
    color("OrangeRed", 0.92)  main_body();
    color("OrangeRed", 0.75)  translate([case_l + 10, 0, 0]) back_cover();
}
