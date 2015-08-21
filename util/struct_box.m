% convenient struct definition for our local shape classifiers
function box = struct_box
box = struct;
box.y = [];
box.x = [];
box.r = [];
box.d = [];
box.invd = [];
box.lay_occd = [];
box.lay_occr = [];
box.lay_occd_conf = [];
box.lay_occr_conf = [];
box.fg_mask = []; % from layers (might be old)
box.bg_mask = [];
box.fg_prob = []; % from colour + shape classification
box.bg_prob = [];
box.fg_prob_colour = [];
box.bg_prob_colour = [];
box.fg_prob_shape = [];
box.bg_prob_shape = [];
box.gmm_change = [];
box.gmm_learn_colour_mask = [];
box.fg_gmm_mu = [];
box.fg_gmm_cov = [];
box.fg_gmm_pi = [];
box.bg_gmm_mu = [];
box.bg_gmm_cov = [];
box.bg_gmm_pi = [];
box.conf = [];
box.conf_colour = [];
box.conf_colour_denom = [];
box.conf_colour_h = [];
box.conf_colour_denom_h = [];
box.conf_mask = [];
box.conf_shape = [];
box.sigma_shape = [];
box.age = [];
end